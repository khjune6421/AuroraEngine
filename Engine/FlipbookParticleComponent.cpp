#include "stdafx.h"
#include "FlipbookParticleComponent.h"

#include "RNG.h"
#include "TimeManager.h"

using namespace std;

REGISTER_TYPE(FlipbookParticleComponent)

void FlipbookParticleComponent::Initialize()
{
	ParticleComponent::Initialize();
	ClampFrame();

	const int maxFrames = GetMaxFrames();
	if (random_start_ && maxFrames > 0) m_currentFrame = RNG::GetInstance().Range(0, maxFrames - 1);
	if (!auto_play_) m_playing = false;
	ApplyFrameToUV();
}

void FlipbookParticleComponent::Update()
{
	if (!m_playing) return;
	if (m_rows <= 0 || m_columns <= 0) return;
	const int maxFrames = GetMaxFrames();
	if (maxFrames <= 1) return;
	if (m_framesPerSecond <= 0.0f) return;
	if (playback_speed_ <= 0.0f) return;

	const float delta_time = TimeManager::GetInstance().GetDeltaTime();
	m_accumulatedTime += delta_time * playback_speed_;

	const float frameDuration = 1.0f / m_framesPerSecond;

	bool updated = false;
	while (m_accumulatedTime >= frameDuration)
	{
		m_accumulatedTime -= frameDuration;
		++m_currentFrame;
		updated = true;

		if (m_currentFrame >= maxFrames)
		{
			if (m_loop) m_currentFrame = 0;
			else
			{
				m_currentFrame = maxFrames - 1;
				m_playing = false;
				if (destroy_on_finish_) LOG("FlipbookParticleComponent: destroy_on_finish_ requested.");
				break;
			}
		}
	}

	if (updated) ApplyFrameToUV();

	ParticleComponent::Update();
}

#ifdef _DEBUG
void FlipbookParticleComponent::RenderImGui()
{
	ParticleComponent::RenderImGui();

	if (ImGui::CollapsingHeader("Flipbook Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool changed = false;
		changed |= ImGui::DragInt("Rows", &m_rows, 1, 1, 128);
		changed |= ImGui::DragInt("Columns", &m_columns, 1, 1, 128);
		changed |= ImGui::DragInt("Start Frame", &m_startFrame, 1, 0, 4095);
		changed |= ImGui::DragInt("End Frame", &m_endFrame, 1, 1, 4096);
		changed |= ImGui::DragFloat("FPS", &m_framesPerSecond, 0.1f, 0.0f, 120.0f);
		changed |= ImGui::DragFloat("Playback Speed", &playback_speed_, 0.1f, 0.1f, 4.0f);
		ImGui::Checkbox("Loop", &m_loop);
		ImGui::Checkbox("Auto Play", &auto_play_);
		ImGui::Checkbox("Random Start", &random_start_);
		ImGui::Checkbox("Destroy On Finish", &destroy_on_finish_);

		const char* playLabel = m_playing ? "Pause" : "Play";
		if (ImGui::Button(playLabel)) m_playing = !m_playing;

		if (ImGui::Button("Restart"))
		{
			const int maxFrames = GetMaxFrames();
			if (random_start_ && maxFrames > 0) m_currentFrame = RNG::GetInstance().Range(0, maxFrames - 1);
			else m_currentFrame = 0;

			m_accumulatedTime = 0.0f;
			m_playing = auto_play_;
			ApplyFrameToUV();
		}

		const int maxFrames = GetMaxFrames();
		if (maxFrames > 0)
		{
			int frameIndex = m_currentFrame;
			if (ImGui::SliderInt("Frame", &frameIndex, 0, maxFrames - 1))
			{
				m_currentFrame = frameIndex;
				ApplyFrameToUV();
			}
		}

		if (changed)
		{
			ClampFrame();
			ApplyFrameToUV();
		}

		ImGui::Text("Current Frame: %d / %d", m_currentFrame + 1, maxFrames);

		if (particle_texture_srv_)
		{
			const ImVec2 previewSize(200.0f, 200.0f);
			const ImTextureID textureId = reinterpret_cast<ImTextureID>(particle_texture_srv_.Get());
			ImGui::Image(textureId, previewSize);

			const ImVec2 p0 = ImGui::GetItemRectMin();
			const ImVec2 p1 = ImGui::GetItemRectMax();
			const ImVec2 imageSize(p1.x - p0.x, p1.y - p0.y);

			const ImVec2 rectMin
			{
				p0.x + uv_buffer_data_.uvOffset.x * imageSize.x,
				p0.y + uv_buffer_data_.uvOffset.y * imageSize.y
			};
			const ImVec2 rectMax
			{
				rectMin.x + uv_buffer_data_.uvScale.x * imageSize.x,
				rectMin.y + uv_buffer_data_.uvScale.y * imageSize.y
			};

			const ImU32 rectColor = IM_COL32(255, 230, 0, 255);
			ImGui::GetWindowDrawList()->AddRect(rectMin, rectMax, rectColor, 0.0f, 0, 2.0f);
		}
		else ImGui::Text("Texture preview: (none)");
	}
}
#endif

nlohmann::json FlipbookParticleComponent::Serialize()
{
	nlohmann::json jsonData = ParticleComponent::Serialize();

	jsonData["flipbookRows"] = m_rows;
	jsonData["flipbookColumns"] = m_columns;
	jsonData["flipbookStartFrame"] = m_startFrame;
	jsonData["flipbookEndFrame"] = m_endFrame;
	jsonData["flipbookFPS"] = m_framesPerSecond;
	jsonData["flipbookLoop"] = m_loop;
	jsonData["flipbookPlaying"] = m_playing;
	jsonData["flipbookCurrentFrame"] = m_currentFrame;
	jsonData["flipbookAutoPlay"] = auto_play_;
	jsonData["flipbookRandomStart"] = random_start_;
	jsonData["flipbookDestroyOnFinish"] = destroy_on_finish_;
	jsonData["flipbookPlaybackSpeed"] = playback_speed_;

	return jsonData;
}

void FlipbookParticleComponent::Deserialize(const nlohmann::json& jsonData)
{
	ParticleComponent::Deserialize(jsonData);

	if (jsonData.contains("flipbookRows")) m_rows = jsonData["flipbookRows"].get<int>();
	if (jsonData.contains("flipbookColumns")) m_columns = jsonData["flipbookColumns"].get<int>();
	if (jsonData.contains("flipbookStartFrame")) m_startFrame = jsonData["flipbookStartFrame"].get<int>();
	if (jsonData.contains("flipbookEndFrame")) m_endFrame = jsonData["flipbookEndFrame"].get<int>();
	if (jsonData.contains("flipbookFPS")) m_framesPerSecond = jsonData["flipbookFPS"].get<float>();
	if (jsonData.contains("flipbookLoop")) m_loop = jsonData["flipbookLoop"].get<bool>();
	if (jsonData.contains("flipbookPlaying")) m_playing = jsonData["flipbookPlaying"].get<bool>();
	if (jsonData.contains("flipbookCurrentFrame")) m_currentFrame = jsonData["flipbookCurrentFrame"].get<int>();
	if (jsonData.contains("flipbookAutoPlay")) auto_play_ = jsonData["flipbookAutoPlay"].get<bool>();
	if (jsonData.contains("flipbookRandomStart")) random_start_ = jsonData["flipbookRandomStart"].get<bool>();
	if (jsonData.contains("flipbookDestroyOnFinish")) destroy_on_finish_ = jsonData["flipbookDestroyOnFinish"].get<bool>();
	if (jsonData.contains("flipbookPlaybackSpeed")) playback_speed_ = jsonData["flipbookPlaybackSpeed"].get<float>();

	m_accumulatedTime = 0.0f;
	ClampFrame();
	ApplyFrameToUV();
}

int FlipbookParticleComponent::GetMaxFrames() const
{
	if (m_rows <= 0 || m_columns <= 0) return 0;
	const int gridFrames = m_rows * m_columns;

	int start = m_startFrame;
	if (start < 0) start = 0;
	if (start >= gridFrames) return 0;

	const int available = gridFrames - start;
	if (m_endFrame <= 0) return available;

	return min(m_endFrame, available);
}

void FlipbookParticleComponent::ClampFrame()
{
	if (m_rows <= 0) m_rows = 1;
	if (m_columns <= 0) m_columns = 1;

	const int gridFrames = m_rows * m_columns;

	if (m_startFrame < 0) m_startFrame = 0;
	if (m_startFrame >= gridFrames) m_startFrame = max(0, gridFrames - 1);

	if (m_endFrame <= 0) m_endFrame = gridFrames - m_startFrame;
	if (m_endFrame <= 0) m_endFrame = 1;

	const int maxFrames = GetMaxFrames();
	if (maxFrames <= 0)
	{
		m_currentFrame = 0;
		return;
	}
	if (m_currentFrame < 0) m_currentFrame = 0;
	if (m_currentFrame >= maxFrames) m_currentFrame = maxFrames - 1;
}

void FlipbookParticleComponent::ApplyFrameToUV()
{
	const int maxFrames = GetMaxFrames();
	if (maxFrames <= 0) return;

	const int columns = max(1, m_columns);
	const int frameIndex = max(0, min(m_currentFrame, maxFrames - 1));

	const int absoluteIndex = m_startFrame + frameIndex;

	const int columnIndex = absoluteIndex % columns;
	const int rowIndex = absoluteIndex / columns;

	uv_buffer_data_.uvScale.x = 1.0f / static_cast<float>(columns);
	uv_buffer_data_.uvScale.y = 1.0f / static_cast<float>(max(1, m_rows));
	uv_buffer_data_.uvOffset.x = static_cast<float>(columnIndex) * uv_buffer_data_.uvScale.x;
	uv_buffer_data_.uvOffset.y = static_cast<float>(rowIndex) * uv_buffer_data_.uvScale.y;
}