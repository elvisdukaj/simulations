module;
#include <GL/glew.h>

#include <SDL3/SDL.h>

#include <numbers>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <vector>

export module vis:graphic.opengl;

import :math;

export namespace vis::opengl {
struct VertexArrayObject {
	VertexArrayObject() {
		glGenVertexArrays(1, &id);
	}
	~VertexArrayObject() {
		if (id != 0)
			glDeleteVertexArrays(1, &id);
	}

	VertexArrayObject(VertexArrayObject&) = delete;
	VertexArrayObject& operator=(VertexArrayObject&) = delete;

	VertexArrayObject(VertexArrayObject&& rhs) {
		std::swap(id, rhs.id);
	}
	VertexArrayObject& operator=(VertexArrayObject&& rhs) {
		std::swap(id, rhs.id);
		return *this;
	}

	void bind() const {
		glBindVertexArray(id);
	}
	static void unbind() {
		glBindVertexArray(0);
	}

	explicit operator GLuint() const {
		return id;
	}

	GLuint id{};
};

struct VertexBufferObject {
	explicit VertexBufferObject(GLenum type) : type{type} {
		glGenBuffers(1, &id);
	}
	~VertexBufferObject() {
		if (id != 0)
			glDeleteBuffers(1, &id);
	}

	VertexBufferObject(VertexBufferObject&) = delete;
	VertexBufferObject& operator=(VertexBufferObject&) = delete;

	VertexBufferObject(VertexBufferObject&& rhs) {
		std::swap(id, rhs.id);
	}

	VertexBufferObject& operator=(VertexBufferObject&& rhs) {
		std::swap(id, rhs.id);
		return *this;
	}

	void bind() const {
		glBindBuffer(type, id);
	}
	void unbind() const {
		glBindBuffer(type, 0);
	}

	template <typename ConstRandomIterator> void data(ConstRandomIterator begin, ConstRandomIterator end, GLenum usage) {
		using value_type = typename std::iterator_traits<ConstRandomIterator>::value_type;

		constexpr auto value_type_size = sizeof(value_type);
		const auto element_count = std::distance(begin, end);
		const auto total_size_in_bytes = element_count * value_type_size;

		std::println("elements: {}, element size: {}, total size: {}", element_count, value_type_size, total_size_in_bytes);

		data(total_size_in_bytes, &(*begin), usage);
	}

	void data(std::size_t size, const void* data, GLenum usage) const {
		glBufferData(type, static_cast<GLsizei>(size), data, usage);
	}

	explicit operator GLuint() const {
		return id;
	}

	GLenum type{GL_VERTEX_ARRAY};
	GLuint id{};
};

enum class ShaderType {
	vertex,
	geometry,
	tesselation_evaluation,
	tesselation_control,
	compute,
	fragment,
};

GLenum to_opengl(ShaderType type) {
	switch (type) {
	case ShaderType::vertex:
		return GL_VERTEX_SHADER;
	case ShaderType::geometry:
		return GL_GEOMETRY_SHADER;
	case ShaderType::tesselation_evaluation:
		return GL_TESS_EVALUATION_SHADER;
	case ShaderType::tesselation_control:
		return GL_TESS_CONTROL_SHADER;
	case ShaderType::compute:
		return GL_COMPUTE_SHADER;
	case ShaderType::fragment:
		return GL_FRAGMENT_SHADER;

	default:
		std::unreachable();
	}
}

class Shader {
public:
	static Shader create(ShaderType type, std::string_view source) {
		return Shader{to_opengl(type), source};
	}

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept : type{other.type}, id{other.id} {
		other.type = GL_INVALID_ENUM;
	}
	Shader& operator=(Shader&& rhs) {
		std::swap(type, rhs.type);
		std::swap(id, rhs.id);
		return *this;
	}

	~Shader() {
		if (type != GL_INVALID_ENUM)
			glDeleteShader(id);
	}

	explicit operator GLuint() const {
		return id;
	}

private:
	explicit Shader(GLenum type, std::string_view source) : type{type}, id{glCreateShader(type)} {
		char const* source_pointer = source.data();
		glShaderSource(id, 1, &(source_pointer), nullptr);
		glCompileShader(id);

		// Check Vertex Shader
		GLint res = GL_FALSE;
		GLint info_log_len = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &res);
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_len);
		if (info_log_len > 0) {
			std::string message;
			message.resize(info_log_len + 1, '\0');
			glGetShaderInfoLog(id, info_log_len, nullptr, message.data());
			std::println("Shader compilation error: {}", message);
		};
	}

private:
	GLenum type;
	GLuint id;
};

class Program {
public:
	static std::optional<Program> create(std::vector<Shader> shaders) {
		return Program{std::move(shaders)};
	}

	Program(const Program&) = delete;
	Program& operator=(const Program&) = delete;

	Program(Program&& other) noexcept : id{other.id} {
		other.id = 0;
	}

	Program& operator=(Program&& rhs) noexcept {
		std::swap(id, rhs.id);
		return *this;
	}

	~Program() {
		if (id != 0)
			glDeleteShader(id);
	}

	explicit operator GLuint() const {
		return id;
	}

	void use() const {
		glUseProgram(id);
	}
	static void unbind() {
		glUseProgram(0);
	}

private:
	explicit Program(std::vector<Shader>&& shaders) : id{glCreateProgram()} {
		for (const auto& shader : shaders) {
			glAttachShader(id, static_cast<GLuint>(shader));
		}

		glLinkProgram(id);

		GLint result = GL_FALSE;
		GLint info_log_len = 0;

		glGetProgramiv(id, GL_LINK_STATUS, &result);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_len);
		if (info_log_len > 0) {
			std::string message;
			message.resize(info_log_len + 1, '\0');
			glGetProgramInfoLog(id, info_log_len, nullptr, message.data());
			std::println("Link error: {}", message);
		}
	}

private:
	GLuint id;
};

class ProgramBuilder {
public:
	ProgramBuilder& add_shader(Shader&& shader) {
		shaders.emplace_back(std::move(shader));
		return *this;
	}

	[[nodiscard]] std::optional<Program> build() {
		return Program::create(std::move(shaders));
	}

private:
	std::vector<Shader> shaders;
};

struct VertexDescription {
	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	const void* pointer;
};

struct DrawDescription {
	GLenum mode;
	GLint first;
	GLsizei vertex_count;
};

struct Vertex {
	vis::vec2 pos;
};

struct Mesh {
	explicit Mesh(std::vector<Vertex> vertexes) : vbo{GL_ARRAY_BUFFER} {
		vao.bind();

		vbo.bind();
		vbo.data(begin(vertexes), end(vertexes), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glDisableVertexAttribArray(0);
	}

	void bind() const noexcept {
		vao.bind();
	}
	void unbind() const noexcept {
		vao.unbind();
	}

private:
	VertexArrayObject vao;
	VertexBufferObject vbo;
};

class GeometryShape {
public:
	vis::vec2 center{};

	friend std::optional<GeometryShape> create_regular_shape(const vis::vec2& center, float radius,
																													 const vis::vec4& color, int num_vertices);
	friend std::optional<GeometryShape> create_rectangle_shape(const vis::vec2& center, const vis::vec2& half_extent);

	void draw(const Program& program) const {
		vao.bind();
		vbo.bind();

		program.use();

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(vertex_description.index, vertex_description.size, vertex_description.type,
													vertex_description.normalized, vertex_description.stride, vertex_description.pointer);

		glDrawArrays(draw_description.mode, draw_description.first, draw_description.vertex_count);
		glDisableVertexAttribArray(0);

		vbo.unbind();
		vao.unbind();

		program.unbind();
	}

private:
	explicit GeometryShape(vis::vec2 center = vis::vec2{0.0f, 0.0f}) : vao{}, vbo{GL_ARRAY_BUFFER} {}

private:
	VertexArrayObject vao;
	VertexBufferObject vbo;
	DrawDescription draw_description;
	VertexDescription vertex_description;
};

std::optional<GeometryShape> create_regular_shape(const vis::vec2& center, float radius, const vis::vec4& color,
																									int num_vertices = 6) {
	GeometryShape shape{center};
	const float theta_step = 2.0f * std::numbers::pi_v<float> / (float)num_vertices;

	using VertexVector = std::vector<Vertex>;

	VertexVector vertexes;
	vertexes.reserve(num_vertices + 2); // plus one for the center, and one for closing
	vertexes.emplace_back(center);
	for (int i = 0; i != num_vertices; i++) {
		auto angle = -theta_step * (float)i;
		vertexes.emplace_back(vis::vec2{std::cos(angle), std::sin(angle)} * radius + center);
	}
	vertexes.emplace_back(vis::vec2{center.x + radius, center.y});

	shape.vao.bind();
	shape.vbo.bind();

	shape.vbo.data(begin(vertexes), end(vertexes), GL_STATIC_DRAW);

	shape.draw_description = DrawDescription{
			.mode = GL_TRIANGLE_FAN,
			.first = 0,
			.vertex_count = (GLsizei)vertexes.size(),
	};
	shape.vertex_description = VertexDescription{
			.index = 0,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = GL_FALSE,
			.stride = 0,
			.pointer = nullptr,
	};
	return shape;
}

std::optional<GeometryShape> create_rectangle_shape(const vis::vec2& center, const vis::vec2& half_extent) {
	GeometryShape shape{center};

	/**            -1,1	A														1,1 B
	 * 							****************|****************
	 * 							*               |               *
	 * 							*               |               *
	 * 						----------------------------------------
	 * 							*               |               *
	 * 							*               |               *
	 * 	   D -1,-1	****************|****************  1,-1 C
	 */

	vis::vec2 up = vis::vec2(0.0f, half_extent.y);
	vis::vec2 down = vis::vec2(0.0f, -half_extent.y);
	vis::vec2 left = vis::vec2(-half_extent.x, 0.0f);
	vis::vec2 right = vis::vec2(half_extent.x, 0.0f);

	std::vector<Vertex> vertexes;
	vertexes.reserve(6);
	vertexes.emplace_back(center + down + left);
	vertexes.emplace_back(center + down + right);
	vertexes.emplace_back(center + up + right);
	vertexes.emplace_back(center + down + left);
	vertexes.emplace_back(center + up + right);
	vertexes.emplace_back(center + up + left);

	shape.vao.bind();
	shape.vbo.bind();

	shape.vbo.data(begin(vertexes), end(vertexes), GL_STATIC_DRAW);

	shape.draw_description = DrawDescription{
			.mode = GL_TRIANGLES,
			.first = 0,
			.vertex_count = (GLsizei)vertexes.size(),
	};
	shape.vertex_description = VertexDescription{
			.index = 0,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = GL_FALSE,
			.stride = 0,
			.pointer = nullptr,
	};

	return shape;
}

void renderer_init() {
	auto glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		throw std::runtime_error("Unable to initialize OpenGL");
	}
}

void renderer_set_clear_color(const vis::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

void renderer_clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}

void renderer_render(SDL_Window* window) {
	SDL_GL_SwapWindow(window);
}

void renderer_set_viewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
}

std::string renderer_print_info() {
	auto renderer = (const char*)glGetString(GL_RENDERER);
	auto version = (const char*)glGetString(GL_VERSION);

	return std::format("viz engine version 0.1\n"
										 "Renderer {}\n"
										 "OpenGL version supported {}",
										 renderer, version);
}

} // namespace vis::opengl