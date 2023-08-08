#pragma once

class Test
{
public:
	static Test* Get();

	static void Create();
	static void Delete();

	void Update();

private:
	Test();
	~Test();

	static Test* instance;

private:
	int callCount = 0;

public:
	void CallCount()
	{
		ImGui::Begin("Print Call Count");
		callCount++;
		ImGui::InputInt("CallCount : ", &callCount);
		ImGui::End();
	}
	void ResetCount() { callCount = 0; }

	void Print(string str)
	{
		ImGui::Begin("Test");
		ImGui::Text(str.c_str());
		ImGui::End();
	}

	void Print(string str, int num)
	{
		ImGui::Begin(str.c_str());
		ImGui::InputInt("", &num);
		ImGui::End();
	}

	void Print(string str, float num)
	{
		ImGui::Begin(str.c_str());
		ImGui::InputFloat("", &num);
		ImGui::End();
	}
};