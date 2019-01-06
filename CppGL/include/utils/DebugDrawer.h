#pragma once

#include "Shader.h"
#include "Loader.h"
#include "Logger.h"

#include <iostream>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

struct DebugPoint final
{
	float x, y, z;
	float r, g, b;
};

class DebugDrawer final : public btIDebugDraw {
	unsigned int _vao, _vbo;
	std::vector<DebugPoint> _lines;
	Shader* _debugShader;
	int _debugMode;
public:
	explicit DebugDrawer(Loader<Shader> shaderLoader): _vao(0), _vbo(0), _debugMode(DBG_DrawWireframe)
	{
		_debugShader = shaderLoader.load("debugline").value();
	}

	void setup() 
	{
		glGenVertexArrays(1, &_vao);
		glGenBuffers(1, &_vbo);
		_debugShader->setup();
	}

	void setDebugMode(int debugMode) override { _debugMode = debugMode;  }

	int getDebugMode() const override
	{
		return _debugMode;
	}
	void drawLine(const btVector3 &from,
		const btVector3 &to,
		const btVector3 &color) override
	{
		DebugPoint start{};
		DebugPoint end{};
		start.x = from.x();
		start.y = from.y();
		start.z = from.z();
		start.r = color.x();
		start.g = color.y();
		start.b = color.z();
		_lines.push_back(start);
		end.x = to.x();
		end.y = to.y();
		end.z = to.z();
		end.r = color.x();
		end.g = color.y();
		end.b = color.z();
		_lines.push_back(end);
	}
	void draw3dText(const btVector3& location, const char* textString) override {}
	void reportErrorWarning(const char *warningString) override
	{
		std::cout << std::string(warningString) << std::endl;
	};
	void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override
	{
		DebugPoint contact{};
		contact.x = pointOnB.x();
		contact.y = pointOnB.y();
		contact.z = pointOnB.z();
		contact.r = color.x();
		contact.g = color.y();
		contact.b = color.z();
		DebugPoint contactEnd{};
		contactEnd.x = pointOnB.x() * (distance / 10.0);
		contactEnd.y = pointOnB.y() * (distance / 10.0);
		contactEnd.z = pointOnB.z() * (distance / 10.0);
		contactEnd.r = color.x();
		contactEnd.g = color.y();
		contactEnd.b = color.z();
		_lines.push_back(contact);
		_lines.push_back(contactEnd);
	}
	void draw(Camera &camera) {
		if (!_lines.empty()) {
			glBindVertexArray(_vao);
			glBindBuffer(GL_ARRAY_BUFFER, _vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _lines.size() * 6, &_lines[0], GL_DYNAMIC_DRAW);

			// Position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
			glEnableVertexAttribArray(0);
			// Colour
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			_debugShader->use();

			const glm::mat4 view = camera.getViewMatrix();
			const glm::mat4 projection = glm::perspective(glm::radians(camera.zoom()), 
				static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
			_debugShader->setMat4("projection", projection);
			_debugShader->setMat4("view", view);

			glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(_lines.size() * 6));

			_lines.clear();
		}
	}
};

