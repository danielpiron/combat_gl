#pragma once

#include <glad/glad.h>

namespace applesauce {

struct Debug {

  static unsigned int quadVAO;
  static unsigned int quadVBO;
  static void renderQuad() {
    if (quadVAO == 0) {
      float quadVertices[] = {
          -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
          0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,
      };
      // setup plane VAO
      glGenVertexArrays(1, &quadVAO);
      glGenBuffers(1, &quadVBO);
      glBindVertexArray(quadVAO);
      glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
                   GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                            (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
  }
};
} // namespace applesauce
