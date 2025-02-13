module;

#include <GL/glew.h>

#include <vector>
#include <numbers>
#include <cmath>

export module vis:mesh;

import :opengl;

export namespace vis::mesh {

    struct Vertex {
        vis::vec2 pos;
    };

    struct VertexDescription {
        GLuint index;
        GLint size;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        const void *pointer;
    };

    struct DrawDescription {
        GLenum mode;
        GLint first;
        GLsizei vertex_count;
    };

    class Mesh {
    public:
        explicit Mesh(std::vector<Vertex> vertexes, VertexDescription vertex_descriptor,
                      DrawDescription draw_descriptor)
                : vertexes{std::move(vertexes)}, vao{},
                  vbo{GL_VERTEX_ARRAY}, vertex_descriptor{vertex_descriptor}, draw_descriptor{draw_descriptor} {
            vao.bind();
            vbo.bind();

            vbo.data(begin(vertexes), end(vertexes), GL_STATIC_DRAW);
        }

        void bind() {
            vao.bind();
            glEnableVertexAttribArray(VERTEX_POS_INDEX);
            glEnableVertexAttribArray(vertex_descriptor.index);
            glVertexAttribPointer(
                    vertex_descriptor.index,
                    vertex_descriptor.size,
                    vertex_descriptor.type,
                    vertex_descriptor.normalized,
                    vertex_descriptor.stride,
                    vertex_descriptor.pointer
            );
            glDisableVertexAttribArray(vertex_descriptor.index);
        }

        void unbind() {
            glDisableVertexAttribArray(vertex_descriptor.index);
            vao.unbind();
        }

        void draw(vis::opengl::Program &program) {
            program.use();
            glDrawArrays(draw_descriptor.mode, draw_descriptor.first, draw_descriptor.vertex_count);
        }

    private:
        static constexpr int VERTEX_POS_INDEX = 0;
        vis::opengl::VertexArrayObject vao;
        vis::opengl::VertexBufferObject vbo;
        std::vector<Vertex> vertexes;
        VertexDescription vertex_descriptor;
        DrawDescription draw_descriptor;
    };

    Mesh create_regular_shape(const vis::vec2 &center, float radius, const vis::vec4 &color,
                              int num_vertices = 6) {
        const float theta_step = 2.0f * std::numbers::pi_v<float> / (float) num_vertices;

        using VertexVector = std::vector<Vertex>;

        VertexVector vertexes;
        vertexes.reserve(num_vertices + 2); // plus one for the center, and one for closing
        vertexes.emplace_back(center);
        for (int i = 0; i != num_vertices; i++) {
            auto angle = -theta_step * (float) i;
            vertexes.emplace_back(vis::vec2{std::cos(angle), std::sin(angle)} * radius + center);
        }
        vertexes.emplace_back(vis::vec2{center.x + radius, center.y});

        auto draw_description = DrawDescription{
                .mode = GL_TRIANGLE_FAN,
                .first = 0,
                .vertex_count = (GLsizei) vertexes.size(),
        };
        auto vertex_description = VertexDescription{
                .index = 0,
                .size = 2,
                .type = GL_FLOAT,
                .normalized = GL_FALSE,
                .stride = 0,
                .pointer = nullptr,
        };
        return Mesh{vertexes, vertex_description, draw_description};
    }

    Mesh create_rectangle_shape(const vis::vec2 &center, const vis::vec2 &half_extent) {
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

        auto draw_description = DrawDescription{
                .mode = GL_TRIANGLES,
                .first = 0,
                .vertex_count = (GLsizei) vertexes.size(),
        };
        auto vertex_description = VertexDescription{
                .index = 0,
                .size = 2,
                .type = GL_FLOAT,
                .normalized = GL_FALSE,
                .stride = 0,
                .pointer = nullptr,
        };

        return Mesh{vertexes, vertex_description, draw_description};
    }

} // namespace vis::mesh