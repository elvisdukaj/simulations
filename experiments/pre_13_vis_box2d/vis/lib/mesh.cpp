module;

#include <GL/glew.h>

#include <cassert>

export module vis:mesh;

import std;
import :opengl;

#ifdef NDEBUG
#define CHECK_LAST_GL_CALL
#else
#define CHECK_LAST_GL_CALL                                                                                             \
	do {                                                                                                                 \
		auto err = glGetError();                                                                                           \
		if (err) {                                                                                                         \
			std::println("[{}][{}:{}] An OpenGL Error occurred", __FILE__, __func__, __LINE__);                              \
			assert(err == 0);                                                                                                \
		}                                                                                                                  \
	} while (false)
#endif

export namespace vis::mesh {

struct Vertex {
	vis::vec2 pos;
	vis::vec4 color;
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

class Mesh {
public:
	explicit Mesh(const std::vector<Vertex>& vertexes, const std::vector<VertexDescription>& vertex_descriptors,
								const DrawDescription& draw_descriptor)
			: vao{}, vbo{GL_ARRAY_BUFFER}, vertex_descriptors{vertex_descriptors}, draw_descriptor{draw_descriptor} {
		vao.bind();
		vbo.bind();

		vbo.data(begin(vertexes), end(vertexes), GL_STATIC_DRAW);

		for (int i = 0; i < draw_descriptor.vertex_count; i++) {
		}

		for (auto& vertex_descriptor : vertex_descriptors) {
			glEnableVertexAttribArray(vertex_descriptor.index);
			CHECK_LAST_GL_CALL;

			glVertexAttribPointer(vertex_descriptor.index, vertex_descriptor.size, vertex_descriptor.type,
														vertex_descriptor.normalized, vertex_descriptor.stride, vertex_descriptor.pointer);
		}

		vbo.unbind();
		vao.unbind();
	}

	void bind() const {
		vao.bind();
	}

	void unbind() const {
		vao.unbind();
	}

	void draw(const vis::opengl::Program& program) const {
		bind();

		program.use();
		glDrawArrays(draw_descriptor.mode, draw_descriptor.first, draw_descriptor.vertex_count);

		unbind();
	}

private:
	vis::opengl::VertexArrayObject vao;
	vis::opengl::VertexBufferObject vbo;
	std::vector<VertexDescription> vertex_descriptors;
	DrawDescription draw_descriptor;
};

Mesh create_regular_shape(const vis::vec2& center, float radius, const vis::vec4& color, int num_vertices = 6) {
	const float theta_step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(num_vertices);

	using VertexVector = std::vector<Vertex>;

	VertexVector vertexes;
	vertexes.reserve(num_vertices + 2); // plus one for the center, and one for closing
	vertexes.emplace_back(center, color);
	for (int i = 0; i != num_vertices; i++) {
		auto angle = -theta_step * static_cast<float>(i);
		vertexes.emplace_back(vis::vec2{std::cos(angle), std::sin(angle)} * radius + center, color);
	};
	vertexes.emplace_back(vis::vec2{center.x + radius, center.y}, color);

	auto draw_description = DrawDescription{
			.mode = GL_TRIANGLE_FAN,
			.first = 0,
			.vertex_count = static_cast<GLsizei>(vertexes.size()),
	};
	auto vertex_descriptions = std::vector<VertexDescription>{
			VertexDescription{
					.index = 0,
					.size = 2,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = nullptr,
			},
			VertexDescription{
					.index = 1,
					.size = 4,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = reinterpret_cast<void*>(sizeof(vis::vec2)),
			},
	};
	return Mesh{vertexes, vertex_descriptions, draw_description};
}

Mesh create_rectangle_shape(const vis::vec2& center, const vis::vec2& half_extent, vis::vec4 color = vis::vec4{}) {
	vis::vec2 up = vis::vec2(0.0f, half_extent.y);
	vis::vec2 down = vis::vec2(0.0f, -half_extent.y);
	vis::vec2 left = vis::vec2(-half_extent.x, 0.0f);
	vis::vec2 right = vis::vec2(half_extent.x, 0.0f);

	std::vector<Vertex> vertexes;
	vertexes.reserve(6);
	vertexes.emplace_back(center + down + left, color);
	vertexes.emplace_back(center + down + right, color);
	vertexes.emplace_back(center + up + right, color);
	vertexes.emplace_back(center + down + left, color);
	vertexes.emplace_back(center + up + right, color);
	vertexes.emplace_back(center + up + left, color);

	auto draw_description = DrawDescription{
			.mode = GL_TRIANGLES,
			.first = 0,
			.vertex_count = static_cast<GLsizei>(vertexes.size()),
	};
	auto vertex_descriptions = std::vector<VertexDescription>{
			VertexDescription{
					.index = 0,
					.size = 2,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = nullptr,
			},
			VertexDescription{
					.index = 1,
					.size = 4,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = reinterpret_cast<void*>(sizeof(vis::vec2)),
			},
	};
	return Mesh{vertexes, vertex_descriptions, draw_description};
}

} // namespace vis::mesh