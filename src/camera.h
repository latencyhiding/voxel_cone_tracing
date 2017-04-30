#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

struct Camera
{
  public:
    Camera(glm::vec3 initial_pos, float pitch, float yaw)
      : m_up(0, 1, 0)
      , m_pos(initial_pos)
      , m_pitch(pitch)
      , m_yaw(yaw)
    {
      calc_front();
    };

    void set_perspective(float lens_angle, float aspect_ratio, float near, float far)
    {
      m_projection = glm::perspective(lens_angle, aspect_ratio, near, far);
    } 

    void calc_front()
    {
      if (m_pitch > 89)
        m_pitch = 89;
      if (m_pitch < -89)
        m_pitch = 89;
      
      m_front.x = cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
      m_front.y = sin(glm::radians(m_pitch));
      m_front.z = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));

      m_front = glm::normalize(m_front);
    }

    void add_rot(float pitch, float yaw)
    {
      m_pitch += pitch;
      m_yaw += yaw;
    }

    void move(float x, float y, float z)
    {
      m_pos += glm::vec3(x * glm::normalize(glm::cross(m_front, m_up)));
      m_pos.y += y;
      m_pos += z * m_front;
    }

    glm::mat4& get_lookat()
    {
      calc_front();      
      m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
      return m_view;
    }

    glm::mat4& get_projection()
    {
      return m_projection;
    }

    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_up;

    float m_pitch;
    float m_yaw;

    glm::mat4 m_projection;
    glm::mat4 m_view;
};
