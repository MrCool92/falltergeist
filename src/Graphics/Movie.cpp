#include "../Game/Game.h"
#include "../Graphics/GLCheck.h"
#include "../Graphics/Movie.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Shader.h"
#include "../ResourceManager.h"

namespace Falltergeist
{
    namespace Graphics
    {
        using Game::Game;

        Movie::Movie()
        {
            // 640x320
            _texture = new Graphics::Texture(640,320);
        }

        Movie::~Movie()
        {
            if (_texture)
            {
                delete _texture;
            }
        }

        unsigned int Movie::width() const
        {
            return _texture->width();
        }

        unsigned int Movie::height() const
        {
            return _texture->height();
        }

        void Movie::loadFromSurface(SDL_Surface* surface)
        {
            _texture->loadFromSurface(surface);
        }

        void Movie::render(int x, int y)
        {
            std::vector<glm::vec2> vertices;
            std::vector<glm::vec2> UV;

            int width = 640;
            int height = 320;

            glm::vec2 vertex_up_left    = glm::vec2( (float)x, (float)y);
            glm::vec2 vertex_up_right   = glm::vec2( (float)(x+width), (float)y);
            glm::vec2 vertex_down_right = glm::vec2( (float)(x+width), (float)(y+height));
            glm::vec2 vertex_down_left  = glm::vec2( (float)x, (float)(y+height));

            vertices.push_back(vertex_up_left   );
            vertices.push_back(vertex_down_left );
            vertices.push_back(vertex_up_right  );
            vertices.push_back(vertex_down_right);

            glm::vec2 uv_up_left    = glm::vec2( 0.0, 0.0 );
            glm::vec2 uv_up_right   = glm::vec2( (float)_texture->width()/(float)_texture->textureWidth(), 0.0 );
            glm::vec2 uv_down_right = glm::vec2( (float)_texture->width()/(float)_texture->textureWidth(), (float)_texture->height()/(float)_texture->textureHeight() );
            glm::vec2 uv_down_left  = glm::vec2( 0.0, (float)_texture->height()/(float)_texture->textureHeight() );

            UV.push_back(uv_up_left   );
            UV.push_back(uv_down_left );
            UV.push_back(uv_up_right  );
            UV.push_back(uv_down_right);

            // TODO: different shader

            ResourceManager::getInstance()->shader("sprite")->use();

            _texture->bind(0);

            GL_CHECK(ResourceManager::getInstance()->shader("sprite")->setUniform("tex",0));

            ResourceManager::getInstance()->shader("sprite")->setUniform("fade",Game::getInstance()->renderer()->fadeColor());

            ResourceManager::getInstance()->shader("sprite")->setUniform("MVP", Game::getInstance()->renderer()->getMVP());

            if (Game::getInstance()->renderer()->renderPath() == Renderer::RenderPath::OGL32)
            {
                GLint curvao;
                glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &curvao);
                GLint vao = Game::getInstance()->renderer()->getVAO();
                if (curvao != vao)
                {
                    GL_CHECK(glBindVertexArray(vao));
                }
            }


            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, Game::getInstance()->renderer()->getVVBO()));

            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_DYNAMIC_DRAW));

            GL_CHECK(glVertexAttribPointer(ResourceManager::getInstance()->shader("sprite")->getAttrib("Position"), 2, GL_FLOAT, GL_FALSE, 0, (void*)0 ));


            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, Game::getInstance()->renderer()->getTVBO()));

            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, UV.size() * sizeof(glm::vec2), &UV[0], GL_DYNAMIC_DRAW));

            GL_CHECK(glVertexAttribPointer(ResourceManager::getInstance()->shader("sprite")->getAttrib("TexCoord"), 2, GL_FLOAT, GL_FALSE, 0, (void*)0 ));

            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Game::getInstance()->renderer()->getEBO()));

            GL_CHECK(glEnableVertexAttribArray(ResourceManager::getInstance()->shader("sprite")->getAttrib("Position")));

            GL_CHECK(glEnableVertexAttribArray(ResourceManager::getInstance()->shader("sprite")->getAttrib("TexCoord")));

            GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 ));

            GL_CHECK(glDisableVertexAttribArray(ResourceManager::getInstance()->shader("sprite")->getAttrib("Position")));

            GL_CHECK(glDisableVertexAttribArray(ResourceManager::getInstance()->shader("sprite")->getAttrib("TexCoord")));

        //    GL_CHECK(glBindVertexArray(0));
        }
    }
}
