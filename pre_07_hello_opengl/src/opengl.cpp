module;
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <numbers>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <vector>

export module vis;

export {
	namespace vis {
	struct VertexArrayObject {
		VertexArrayObject() { glGenVertexArrays(1, &id); }
		~VertexArrayObject() { glDeleteVertexArrays(1, &id); }

		void bind() const { glBindVertexArray(id); }
		static void unbind() { glBindVertexArray(0); }

		explicit operator GLuint() const { return id; }

		GLuint id{};
	};

	struct VertexBufferObject {
		explicit VertexBufferObject(GLenum type) : type{type} { glGenBuffers(1, &id); }
		~VertexBufferObject() { glDeleteBuffers(1, &id); }

		void bind() const { glBindBuffer(type, id); }
		void unbind() const { glBindBuffer(type, 0); }

		void data(GLsizeiptrARB size, const void* data, GLenum usage) const { glBufferData(type, size, data, usage); }

		explicit operator GLuint() const { return id; }

		GLenum type{GL_VERTEX_ARRAY};
		GLuint id{};
	};

	struct Vertex {
		glm::vec2 pos;
	};

	// (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

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
		std::size_t vertex_count;
	};

	class GeometryShape {
	public:
		glm::vec2 center{};
		VertexArrayObject vao;
		VertexBufferObject vbo;

		void draw() const {
			vbo.bind();
			glVertexAttribPointer(0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
														2, // size
														GL_FLOAT, // type
														GL_FALSE, // normalized?
														0,				// stride
														nullptr); // array buffer offset

			glDrawArrays(GL_TRIANGLES, draw_description.mode, (int)draw_description.vertex_count);
		}

	private:
		explicit GeometryShape(glm::vec2 center = glm::vec2{0.0f, 0.0f}) : vao{}, vbo{GL_ARRAY_BUFFER} {}
		friend GeometryShape create_regular_shape(const glm::vec2& center, float radius, const glm::vec4& color,
																							int num_vertices);

	private:
		DrawDescription draw_description;
		VertexDescription vertex_description;
	};

	GeometryShape create_regular_shape(const glm::vec2& center, float radius, const glm::vec4& color,
																		 int num_vertices = 6) {
		GeometryShape shape{center};
		const float theta_step = 2.0f * std::numbers::pi_v<float> / (float)num_vertices;

		std::vector<Vertex> vertexes;
		vertexes.reserve(num_vertices + 1); // plus one for the center
		vertexes.emplace_back(center);
		for (int i = 0; i != num_vertices; i++) {
			auto angle = std::numbers::pi_v<float> * 2.0f - (theta_step * (float)i);
			auto x = std::cos(angle * radius + center.x);
			auto y = std::sin(angle * radius + center.y);

			vertexes.emplace_back(glm::vec2{x, y});
		}

		shape.vao.bind();
		shape.vbo.data(sizeof(vertexes), vertexes.data(), GL_STATIC_DRAW);
		shape.draw_description = DrawDescription{
				.mode = GL_TRIANGLE_FAN,
				.vertex_count = vertexes.size(),
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

	class Shader {
	public:
		static Shader create(GLenum type, std::string_view source) { return Shader{type, source}; }

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		Shader(Shader&& other) noexcept : type{other.type}, id{other.id} { other.type = GL_INVALID_ENUM; }
		Shader& operator=(Shader&& rhs) {
			std::swap(type, rhs.type);
			std::swap(id, rhs.id);
			return *this;
		}

		~Shader() {
			if (type != GL_INVALID_ENUM)
				glDeleteShader(id);
		}

		explicit operator GLuint() const { return id; }

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
		static std::optional<Program> create(std::vector<Shader> shaders) { return Program{std::move(shaders)}; }

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		Program(Program&& other) noexcept : id{other.id} { other.id = 0; }

		Program& operator=(Program&& rhs) noexcept {
			std::swap(id, rhs.id);
			return *this;
		}

		~Program() {
			if (id != 0)
				glDeleteShader(id);
		}

		explicit operator GLuint() const { return id; }

		void use() const { glUseProgram(id); }

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

		[[nodiscard]] std::optional<Program> build() { return Program::create(std::move(shaders)); }

	private:
		std::vector<Shader> shaders;
	};

	} // namespace vis
}