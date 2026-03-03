#include "stdafx.h"

#include "GameObjectBase.h"
#include "ListenerComponent.h"

#include "SoundManager.h"
#include "TimeManager.h"

#include "Shared/Config/Option.h"
#include "SceneManager.h"
#include "SceneBase.h"

constexpr size_t ChannelCount = 64; //profiling

void SoundManager::Initialize()
{
	m_RhythmOffSet = Config::travelTime;

	m_Volume_Main = Config::Master_Volume;
	m_Volume_BGM = Config::BGM_Volume;
	m_Volume_AMB = Config::AMB_Volume;
	m_Volume_SFX = Config::SFX_Volume;
	m_Volume_UI = Config::UI_Volume;

	FMOD_RESULT result;
	if (!m_CoreSystem)
	{
		result = FMOD::System_Create(&m_CoreSystem);
		if (result != FMOD_OK) { FMOD_LOG(result); FMOD_ASSERT(result); return; }

#ifdef _DEBUG
		result = m_CoreSystem->init(ChannelCount,
			FMOD_INIT_NORMAL |
			FMOD_INIT_VOL0_BECOMES_VIRTUAL |
			FMOD_INIT_PROFILE_ENABLE
			, nullptr);
#else
		result = m_CoreSystem->init(ChannelCount,
			FMOD_INIT_NORMAL |
			FMOD_INIT_VOL0_BECOMES_VIRTUAL
			, nullptr);

#endif
		if (result != FMOD_OK) { FMOD_LOG(result); FMOD_ASSERT(result); return; }

		m_CoreSystem->getMasterChannelGroup(&m_MainGroup);

		if (!m_BGMGroup) m_CoreSystem->createChannelGroup("BGM", &m_BGMGroup);
		if (!m_AMBGroup) m_CoreSystem->createChannelGroup("AMB", &m_AMBGroup);
		if (!m_SFXGroup) m_CoreSystem->createChannelGroup("SFX", &m_SFXGroup);
		if (!m_UIGroup)  m_CoreSystem->createChannelGroup("UI", &m_UIGroup);

		m_MainGroup->addGroup(m_BGMGroup);
		m_MainGroup->addGroup(m_AMBGroup);
		m_MainGroup->addGroup(m_SFXGroup);
		m_MainGroup->addGroup(m_UIGroup);

		ConvertBGMSource();
		ConvertSFXSource();
		ConvertUISource();

		if (CheckMainBGMBeatver())
		{
			LOG_ERROR("Not Found _Beat Source");
		}

		LOG("##BGM_List##");
		for (auto& n : BGM_List)
		{
			LOG(n.first);
			CreateNodeData(n.first);
		}
		LOG("##BGM_End##");
		LOG("##SFX_List##");
		for (auto& n : SFX_List)
		{
			LOG(n.first);
		}
		LOG("##SFX_End##");
		LOG("##UI_List##");
		for (auto& n : UI_List)
		{
			LOG(n.first);
		}
		LOG("##UI_End##");
		SoundManager::GetInstance().LoadNodeData();


		m_CoreSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &m_lowpass);
		m_lowpass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000.0f);
		m_BGMGroup->addDSP(0, m_lowpass);

		m_CurrentTrackName = "";
		m_CurrentNodeDataName = "";


#ifdef _DEBUG
		SoundManager::GetInstance().Main_BGM_Shot(Config::Tutori_BGM, 0.0f);
#endif

		m_CoreSystem->update();
	}
}

void SoundManager::Update()
{
	m_CoreSystem->update();

	UpdateAudioClock();

	UpdateLowpass();

	UpdateNodeIndex();
	UpdateUINodeIndexAndGenerated();
	UpdateUINodeDestroyed();
	ConsumeNodeGenerated();
	ConsumeNodeDestroyed();

	FMOD::ChannelGroup* master;
	m_CoreSystem->getMasterChannelGroup(&master);

	//std::cout << m_rhythmTimerIndex << " : index "
	//	<< m_NodeData[m_rhythmTimerIndex].first << " : startTime "
	//	<< m_NodeData[m_rhythmTimerIndex].second << " : EndTime " << std::endl;

}

void SoundManager::Stop_ChannelGroup()
{
	if (m_BGMGroup) m_BGMGroup->stop();
	if (m_SFXGroup) m_SFXGroup->stop();
	if (m_UIGroup)  m_UIGroup->stop();
}
void SoundManager::Release_ChannelGroup()
{
	if (m_BGMGroup) m_BGMGroup->release();
	if (m_SFXGroup) m_SFXGroup->release();
	if (m_UIGroup)  m_UIGroup->release();
}

void SoundManager::Finalize()
{
	Stop_ChannelGroup();
	Release_ChannelGroup();

	if (m_CoreSystem)
	{
		m_CoreSystem->close();
		m_CoreSystem->release();
	}
}

void SoundManager::ConvertBGMSource()
{
	bool hasfile = false;

	const std::filesystem::path BGMDirectory = "../Asset/Sound/BGM/";

	if (std::filesystem::exists(BGMDirectory))
	{
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(BGMDirectory))
		{
			if (!dirEntry.is_regular_file())
				continue;

			hasfile = true;

			std::string fileName = std::filesystem::path(dirEntry).stem().string();

			FMOD::Sound* temp;
			std::string fullPath = dirEntry.path().string();
			m_CoreSystem->createSound(fullPath.c_str(), FMOD_CREATESAMPLE |
				FMOD_LOOP_NORMAL |
				FMOD_2D |
				FMOD_ACCURATETIME, nullptr, &temp);

			FMOD_SOUND_FORMAT format;
			temp->getFormat(nullptr, &format, nullptr, nullptr);
			if (format != FMOD_SOUND_FORMAT_PCM16)
			{
				std::string err{};
				switch (format)
				{
				case FMOD_SOUND_FORMAT_NONE:
					err = "FMOD_SOUND_FORMAT_NONE";
					break;
				case FMOD_SOUND_FORMAT_PCM8:
					err = "FMOD_SOUND_FORMAT_PCM8";
					break;
				case FMOD_SOUND_FORMAT_PCM16:
					err = "FMOD_SOUND_FORMAT_PCM16";
					break;
				case FMOD_SOUND_FORMAT_PCM24:
					err = "FMOD_SOUND_FORMAT_PCM24";
					break;
				case FMOD_SOUND_FORMAT_PCM32:
					err = "FMOD_SOUND_FORMAT_PCM32";
					break;
				case FMOD_SOUND_FORMAT_PCMFLOAT:
					err = "FMOD_SOUND_FORMAT_PCMFLOAT";
					break;
				case FMOD_SOUND_FORMAT_BITSTREAM:
					err = "FMOD_SOUND_FORMAT_BITSTREAM";
					break;
				case FMOD_SOUND_FORMAT_MAX:
					err = "FMOD_SOUND_FORMAT_MAX";
					break;
				case FMOD_SOUND_FORMAT_FORCEINT:
					err = "FMOD_SOUND_FORMAT_FORCEINT";
					break;
				}
				LOG_ERROR("Error format incompatible, Name: " << fileName << " Format: " << err);
				continue;
			}

			BGM_List.emplace(fileName, temp);
		}

		if (!hasfile)
		{
			LOG_ERROR("BGM resource not found");
		}
	}
	else
	{
		LOG_ERROR("BGM path not found");
	}
}

void SoundManager::ConvertSFXSource()
{
	bool hasfile = false;

	const std::filesystem::path SFXDirectory = "../Asset/Sound/SFX/";

	if (std::filesystem::exists(SFXDirectory))
	{
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(SFXDirectory))
		{
			if (!dirEntry.is_regular_file())
				continue;

			hasfile = true;

			std::string fileName = std::filesystem::path(dirEntry).stem().string();

			FMOD::Sound* temp;
			std::string fullPath = dirEntry.path().string();
			m_CoreSystem->createSound(fullPath.c_str(), FMOD_DEFAULT | FMOD_3D, nullptr, &temp);

			SFX_List.emplace(fileName, temp);

		}

		if (!hasfile)
		{
			LOG_ERROR("SFX resource not found");
		}
	}
	else
	{
		LOG_ERROR("SFX path not found");
	}
}

void SoundManager::ConvertUISource()
{
	bool hasfile = false;

	const std::filesystem::path UIDirectory = "../Asset/Sound/UI/";

	if (std::filesystem::exists(UIDirectory))
	{
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(UIDirectory))
		{
			if (!dirEntry.is_regular_file())
				continue;

			hasfile = true;

			std::string fileName = std::filesystem::path(dirEntry).stem().string();

			FMOD::Sound* temp;
			std::string fullPath = dirEntry.path().string();
			m_CoreSystem->createSound(fullPath.c_str(), FMOD_DEFAULT | FMOD_2D, nullptr, &temp);

			UI_List.emplace(fileName, temp);

		}
		if (!hasfile)
		{
			LOG_ERROR("UI resource not found");
		}
	}
	else
	{
		LOG_ERROR("UI path not found");
	}
}

bool SoundManager::CheckMainBGMBeatver()
{
	for (auto& m : BGM_List)
	{
		if (m.first.find("_Beat"))
		{
			return true;
		}
		
		return false;
	}
	return false;
}

static float Hann(int n, int N)
{
	return 0.5f * (1.0f - cosf(2.0f * DirectX::XM_PI * static_cast<float>(n) / static_cast<float>(N - 1)));
}

struct Segment
{
	float start;
	float end;
};

void SoundManager::CreateNodeData(const std::string& filename)
{
	auto it = BGM_List.find(filename);
	if (it == BGM_List.end())
		return;

	std::string path = "../Asset/BeatMapData/";
	std::string ext = "_nodes.json";

	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directories(path);
	}

	std::ifstream file(path + filename + ext);

	if (file.is_open())
		return;

	FMOD::Sound* sound = it->second;

	FMOD_SOUND_FORMAT format;
	int channels, bits;
	sound->getFormat(nullptr, &format, &channels, &bits);

	float sampleRate = 0.0f;
	sound->getDefaults(&sampleRate, nullptr);

	unsigned int pcmBytes = 0;
	sound->getLength(&pcmBytes, FMOD_TIMEUNIT_PCMBYTES);

	void* ptr1 = nullptr;
	void* ptr2 = nullptr;
	unsigned int len1 = 0, len2 = 0;

	sound->lock(0, pcmBytes, &ptr1, &ptr2, &len1, &len2);

	std::vector<float> pcmFloat;

	if (format == FMOD_SOUND_FORMAT_PCM16)
	{
		int totalSamples = len1 / sizeof(int16_t);
		int16_t* pcm16 = static_cast<int16_t*>(ptr1);

		pcmFloat.resize(totalSamples / channels);

		if (channels == 2)
		{
			for (size_t i = 0; i < pcmFloat.size(); i++)
			{
				pcmFloat[i] =
					(pcm16[i * 2] + pcm16[i * 2 + 1]) / 65536.0f;
			}
		}
		else if (channels == 1)
		{
			for (size_t i = 0; i < pcmFloat.size(); i++)
			{
				pcmFloat[i] = pcm16[i] / 32768.0f;
			}
		}
	}
	else
	{
		std::string msg = "\"" + it->first + "\"" + " format is Not PCM16";
		MessageBoxA(nullptr, msg.c_str(), "Error", MB_OK | MB_ICONERROR);
	}

	sound->unlock(ptr1, ptr2, len1, len2);

	if (pcmFloat.empty())
		return;

	const int fftSize = 1024;
	const int hopSize = 512;

	float* fftIn = (float*)fftwf_malloc(sizeof(float) * fftSize);
	fftwf_complex* fftOut =
		(fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * (fftSize / 2 + 1));

	fftwf_plan fftPlan =
		fftwf_plan_dft_r2c_1d(
			fftSize,
			fftIn,
			fftOut,
			FFTW_MEASURE
		);

	float minHz = 40.0f;
	float maxHz = 110.0f;

	int minBin = (int)(minHz * fftSize / sampleRate);
	int maxBin = (int)(maxHz * fftSize / sampleRate);

	bool active = false;
	float startTime = 0.0f;
	float onsetEnergy = 0.0f;
	float prevEnergy = 0.0f;

	float deltaSum = 0.0f;
	int deltaCount = 0;

	float deltaMultiplier = 1.1f;
	float decayRatio = 0.3f;

	float prev2Energy = 0.0f;
	float prev1Energy = 0.0f;

	std::vector<Segment> segments;

	const float kickEnergyThreshold = 5000.0f;
	const float kickLength = 0.10f;
	const float minInterval = 0.10f;

	int warmupFrames = 5;
	int frameIndex = 0;

	for (size_t offset = 0;
		offset + fftSize <= pcmFloat.size();
		offset += hopSize)
	{
		frameIndex++;

		for (int i = 0; i < fftSize; i++)
			fftIn[i] = pcmFloat[offset + i] * Hann(i, fftSize);

		fftwf_execute(fftPlan);

		float energy = 0.0f;
		for (int k = minBin; k <= maxBin; k++)
		{
			float r = fftOut[k][0];
			float im = fftOut[k][1];
			energy += r * r + im * im;
		}

		if (frameIndex <= warmupFrames)
		{
			prev2Energy = prev1Energy;
			prev1Energy = energy;
			continue;
		}

		float timeSec = (float)offset / sampleRate;

		bool isKickPeak =
			(prev1Energy > prev2Energy) &&
			(prev1Energy > energy) &&
			(prev1Energy > kickEnergyThreshold);

		if (isKickPeak)
		{
			float kickStart =
				timeSec - (fftSize * 0.5f / sampleRate);
			float kickEnd = kickStart + kickLength;

			if (segments.empty() ||
				kickStart - segments.back().start > minInterval)
			{
				segments.push_back({ kickStart, kickEnd });
			}
		}

		prev2Energy = prev1Energy;
		prev1Energy = energy;
	}

	if (active)
	{
		float endTime = (float)pcmFloat.size() / sampleRate;
		segments.push_back({ startTime, endTime });
	}

	std::vector<Segment> filtered;
	for (auto& s : segments)
	{
		if (s.end - s.start >= 0.03f) // 30ms
			filtered.push_back(s);
	}

	fftwf_destroy_plan(fftPlan);
	fftwf_free(fftIn);
	fftwf_free(fftOut);

	nlohmann::json root;
	root["band"] = "kick";
	root["rangeHz"] = { minHz, maxHz };
	root["segments"] = nlohmann::json::array();

	if (filename.find("deadeye") != std::string::npos)
	{
		for (auto& s : filtered)
		{
			root["segments"].push_back({
				{ "start", s.start },
				{ "end",   s.end }
				});
		}
	}
	else
	{
		for (auto& s : filtered)
		{
			root["segments"].push_back({
				{ "start", s.start - m_RhythmOffSet },
				{ "end",   s.end - m_RhythmOffSet }
				});
		}
	}

	std::string outPath = "../Asset/BeatMapData/" + filename + "_nodes.json";
	std::ofstream out(outPath);
	out << root.dump(4);
	out.close();
}

void SoundManager::LoadNodeData()
{
	/*if (strcmp(m_CurrentTrackName.c_str(), "Invaild") == 0)
	{
		LOG_ERROR("Invaild Node Data");
		return;
	}*/

	for (auto& m : BGM_List)
	{
		std::string songName = m.first;

		std::string path = "../Asset/BeatMapData/" + songName + "_nodes.json";
		std::ifstream in(path);
		nlohmann::json j;
		in >> j;

		for (auto& seg : j["segments"])
		{
			m_NodeData[songName].push_back({ seg["start"], seg["end"] });
		}
	}
}

void SoundManager::UpdateNodeIndex() //raw time
{
	if (m_rhythmTimerIndex < m_NodeData[m_CurrentNodeDataName].size() &&
		m_NodeData[m_CurrentNodeDataName][m_rhythmTimerIndex].second + m_RhythmOffSet < GetAudioTime())
	{
		m_rhythmTimerIndex++;

		//std::cout << "RTIndex : " << m_rhythmTimerIndex << std::endl;
	}
}

void SoundManager::UpdateUINodeIndexAndGenerated()
{
	if (m_rhythmUIIndex < m_NodeData[m_CurrentNodeDataName].size() &&
		m_NodeData[m_CurrentNodeDataName][m_rhythmUIIndex].first < GetAudioTime())
	{
		m_rhythmUIIndex++;
		m_OnNodeGenerated = true;

		//std::cout << "UIndex : " << m_rhythmUIIndex << std::endl;
	}
}

InputType SoundManager::CheckRhythm(float correction)
{
	if (m_CurrentNodeDataName.empty()) return InputType::Fatal;
	if (m_CurrentNodeDataName == "") return InputType::Fatal;
	if (m_isBeatConsumed) return InputType::Fatal;

	const float time = GetAudioTime() + Config::BeatHumanOffset;
	// c - s < time > s = Early
	if (m_NodeData[m_CurrentNodeDataName][m_rhythmTimerIndex].first - correction + m_RhythmOffSet < time && m_NodeData[m_CurrentNodeDataName][m_rhythmTimerIndex].first + m_RhythmOffSet > time)
	{
		m_isBeatConsumed = true;
		return InputType::Early;
	}
	// s < time > e  = perfect
	else if (m_NodeData[m_CurrentNodeDataName][m_rhythmTimerIndex].first + m_RhythmOffSet < time && m_NodeData[m_CurrentNodeDataName][m_rhythmTimerIndex].second + m_RhythmOffSet > time)
	{
		m_isBeatConsumed = true;
		return InputType::Perfect;
	}
	// e < time > e + c = late
	else if (m_NodeData[m_CurrentNodeDataName][m_rhythmTimerIndex].second + m_RhythmOffSet < time && m_NodeData[m_CurrentNodeDataName][m_rhythmTimerIndex].second + correction + m_RhythmOffSet > time)
	{
		m_isBeatConsumed = true;
		return InputType::Late;
	}
	else
	{
		m_isBeatConsumed = true;
		return InputType::Miss;
	}
}

void SoundManager::UpdateUINodeDestroyed()
{
	if (m_CurrentNodeDataName.empty() || m_CurrentNodeDataName == "" && m_rhythmDestroyIndex > m_NodeData[m_CurrentNodeDataName].size() -1) return;

	if (m_NodeData[m_CurrentNodeDataName][m_rhythmDestroyIndex].first + m_RhythmOffSet < GetAudioTime())
	{
		static int cnt = 0;

		m_rhythmDestroyIndex++;
		m_isBeatConsumed = false;
		m_OnNodeDestroyed = true;
		return;
	}
}

void SoundManager::Main_BGM_Shot(const std::string filename, float delay)
{
	auto it = BGM_List.find(filename);
	if (it == BGM_List.end())
	{
		LOG_ERROR("Cannot play: invalid track name." << filename);
		CheckResult(-1, "invalid track name");
	}

	it->second->setMode(FMOD_LOOP_NORMAL);
	it->second->setLoopCount(-1);

	m_CurrentTrackName = it->first;
	m_CurrentNodeDataName = it->first + "_Beat";
	m_rhythmTimerIndex = 0;
	m_rhythmUIIndex = 0;
	m_rhythmDestroyIndex = 0;

	m_CoreSystem->playSound(it->second, m_BGMGroup, true, &m_BGMChannel1);
	m_BGMChannel1->setChannelGroup(m_BGMGroup);

	unsigned long long nowDSP;
	m_BGMGroup->getDSPClock(&nowDSP, nullptr);

	m_CoreSystem->getSoftwareFormat(&m_DspSampleRate, nullptr, nullptr);

	unsigned long long delaySamples =
		static_cast<unsigned long long>(delay * m_DspSampleRate);

	m_BGMStartDSP = nowDSP + delaySamples;

	m_PrevAudioTime = 0.0f;
	m_AudioTime = 0.0f;
	m_AudioDeltaTime = 0.0f;

	m_BGMChannel1->setDelay(m_BGMStartDSP, 0, false);

	m_BGMChannel1->setPaused(false);
	m_BGMChannel1->setVolume(m_Volume_BGM);
}

void SoundManager::Sub_BGM_Shot(const std::string filename, float delay)
{
	auto it = BGM_List.find(filename);
	if (it == BGM_List.end())
	{
		LOG_ERROR("Cannot play: invalid track name : " << filename);
		CheckResult(-1, "invalid track name");
	}

	m_CoreSystem->playSound(it->second, m_BGMGroup, true, &m_BGMChannel2);

	unsigned long long nowDSP;
	m_MainGroup->getDSPClock(&nowDSP, nullptr);

	m_CoreSystem->getSoftwareFormat(&m_DspSampleRate, nullptr, nullptr);

	unsigned long long delaySamples =
		static_cast<unsigned long long>(delay * m_DspSampleRate);

	m_BGMStartDSP = nowDSP + delaySamples;

	m_BGMChannel2->setDelay(m_BGMStartDSP, 0, false);

	m_BGMChannel2->setPaused(false);
}

void SoundManager::Ambience_Shot(const std::string filename)
{
	if (filename.empty())
	{
		LOG_ERROR("Cannot play: Ambience filename is empty");
		return;
	}

	auto it = BGM_List.find(filename);
	if (it == BGM_List.end())
	{
		LOG_ERROR("Cannot play: invalid track name : " << filename);
		CheckResult(-1, "invalid track name");
	}
	it->second->setMode(FMOD_LOOP_NORMAL);
	it->second->setLoopCount(-1);

	m_CoreSystem->playSound(it->second, m_BGMGroup, false, &m_AmbienceCh);
}

void SoundManager::SFX_Shot(const DirectX::XMVECTOR pos, const std::string filename)
{
	FMOD::Channel* pChannel = nullptr;

	auto it = SFX_List.find(filename);

	if (it != SFX_List.end())
	{
		m_CoreSystem->playSound(it->second, m_SFXGroup, false, &pChannel);
		FMOD_VECTOR tempPos = ToFMOD(pos);
		FMOD_VECTOR vel{ 0,0,0 };
		pChannel->set3DAttributes(&tempPos, &vel);
	}
	else
	{
		LOG("Not Found SFX FileName : " << filename);
		return;
	}
}

void SoundManager::UI_Shot(const std::string filename)
{
	FMOD::Channel* pChannel = nullptr;

	auto it = UI_List.find(filename);

	if (it != UI_List.end())
	{
		m_CoreSystem->playSound(it->second, m_UIGroup, false, &pChannel);
	}
}

void SoundManager::FadeIn(FMOD::Channel* chan, float sec)
{
	if (!chan) return;

	FMOD::System* sys = nullptr;
	chan->getSystemObject(&sys);

	FMOD::ChannelGroup* master = nullptr;
	sys->getMasterChannelGroup(&master);

	int rate = 0;
	sys->getSoftwareFormat(&rate, nullptr, nullptr);

	unsigned long long dspNow = 0;
	master->getDSPClock(&dspNow, nullptr);

	chan->setPaused(true);

	chan->addFadePoint(dspNow, 0.0f);
	chan->addFadePoint(dspNow + (unsigned long long)(rate * sec),
		GetVolume_Main() * GetVolume_BGM());

	chan->setPaused(false);
}


void SoundManager::FadeOut(FMOD::Channel* chan, float sec, bool stopAfter)
{
	if (!chan) return;

	FMOD::System* sys = nullptr;
	chan->getSystemObject(&sys);

	FMOD::ChannelGroup* master = nullptr;
	sys->getMasterChannelGroup(&master);

	int rate = 0;
	sys->getSoftwareFormat(&rate, nullptr, nullptr);

	unsigned long long dspNow = 0;
	master->getDSPClock(&dspNow, nullptr);

	chan->addFadePoint(dspNow, GetVolume_Main() * GetVolume_BGM());
	chan->addFadePoint(dspNow + (unsigned long long)(rate * sec), 0.0f);

	if (stopAfter)
		chan->setDelay(0, dspNow + (unsigned long long)(rate * sec), true);
}


//float SoundManager::GetCurrentPlaybackTime() //position ver
//{
//	if (!m_BGMChannel1)
//		return 0.0f;
//
//	unsigned int nowSongTime;
//	m_BGMChannel1->getPosition(&nowSongTime, FMOD_TIMEUNIT_MS);
//
//	/*if (nowSongTime < m_MainBGM_StartTime)
//		return 0.0f;*/
//
//	//int dspSampleRate = 0;
//	//m_CoreSystem->getSoftwareFormat(&dspSampleRate, nullptr, nullptr);
//
//	/*double songTime =
//		(double)(nowDSP - m_MainBGM_StartTime)
//		/ (double)dspSampleRate;*/
//
//	return static_cast<float>(nowSongTime) / 1000.0f;
//}

void SoundManager::UpdateAudioClock()
{


	unsigned long long nowDSP;
	m_BGMGroup->getDSPClock(&nowDSP, nullptr);

	if (nowDSP <= m_BGMStartDSP)
	{
		m_AudioTime = 0.0f;
		m_AudioDeltaTime = 0.0f;
		m_PrevAudioTime = 0.0f;
		return;
	}

	const float now =
		static_cast<float>(nowDSP - m_BGMStartDSP) /
		static_cast<float>(m_DspSampleRate);

	const float nowF = static_cast<float>(now);

	float dt = nowF - m_PrevAudioTime;

	if (dt < 0.0f)  dt = 0.0f;
	if (dt > 0.25f) dt = 0.25f;

	m_AudioDeltaTime = dt;
	m_PrevAudioTime = nowF;
	m_AudioTime = nowF;
}

void SoundManager::Pause()
{
	if (m_isPaused) return;
	if (!m_MainGroup) return;

	m_MainGroup->getDSPClock(&m_pauseDSP, nullptr);
	m_MainGroup->setPaused(true);
	m_isPaused = true;
}

void SoundManager::Resume()
{
	if (!m_isPaused) return;

	unsigned long long nowDSP;
	m_BGMGroup->getDSPClock(&nowDSP, nullptr);

	unsigned long long pausedDuration = nowDSP - m_pauseDSP;

	// 핵심: 시작 기준을 뒤로 밀어줌
	m_BGMStartDSP += pausedDuration;

	m_MainGroup->setPaused(false);
	m_isPaused = false;
}

FMOD_VECTOR SoundManager::ToFMOD(DirectX::XMVECTOR vector)
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMStoreFloat3(&pos, vector);

	FMOD_VECTOR FMOD_pos;
	FMOD_pos.x = pos.x;
	FMOD_pos.y = pos.y;
	FMOD_pos.z = pos.z;

	return FMOD_pos;
}

void SoundManager::ConsumeNodeGenerated()
{
	if (m_OnNodeGenerated)
	{
		NotifyNodeGenerated();
		m_OnNodeGenerated = false;
	}
}

void SoundManager::ConsumeNodeDestroyed()
{
	if (m_OnNodeDestroyed)
	{
		NotifyNodeDestroyed();
		m_OnNodeDestroyed = false;
	}
}

//void SoundManager::AddNodeChangedListener(std::function<void()> cb)
//{
//	m_NodeChangedListeners.push_back(cb);
//}

void SoundManager::AddNodeGeneratedListenerOnce(std::function<void()> cb)
{
	m_NodeGeneratedListenerOnce.push_back(cb);
}

void SoundManager::AddNodeDestroyedListenerOnce(std::function<bool()> cb)
{
	m_NodeDestroyedListenerOnce.push_back(cb);
}

void SoundManager::NotifyNodeGenerated()
{
	for (auto& cb : m_NodeGeneratedListenerOnce)
	{
		if (SceneManager::GetInstance().GetCurrentScene()->GetType() == "TestScene")
		{
			cb();
		}
		else
		{
			m_NodeGeneratedListenerOnce.clear();
		}
	}

	m_NodeGeneratedListenerOnce.clear();
}

void SoundManager::NotifyNodeDestroyed()
{
	auto it = m_NodeDestroyedListenerOnce.begin();

	while (it != m_NodeDestroyedListenerOnce.end())
	{
		if ((*it)())
			it = m_NodeDestroyedListenerOnce.erase(it);
		else
			++it;
	}
}

void SoundManager::UpdateLowpass()
{
	float delta = TimeManager::GetInstance().GetNSDeltaTime();
	float speed = 60000.0f;

	if (m_IsLowpass)
	{
		m_lowpassCutOff -= speed * delta;
		m_lowpassCutOff = std::max(m_lowpassCutOff, 800.0f);

		m_lowpass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, m_lowpassCutOff);
	}
	else
	{
		m_lowpassCutOff += speed * delta;
		m_lowpassCutOff = std::min(m_lowpassCutOff, 22000.0f);

		m_lowpass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, m_lowpassCutOff);
	}

	//std::cout << m_IsLowpass << std::endl;
}

bool SoundManager::CheckBGMEnd()
{
	unsigned int pos = 0;
	unsigned int len = 0;

	if (!m_BGMChannel1)
		return false;

	m_BGMChannel1->getPosition(&pos, FMOD_TIMEUNIT_MS);
	FMOD::Sound* temp = nullptr;
	m_BGMChannel1->getCurrentSound(&temp);
	temp->getLength(&len, FMOD_TIMEUNIT_MS);

	if (pos >= len)
	{
		return true;
	}

	return false;
}
//void SoundManager::UpdateSoundResourceUsage()
//{
//	m_CoreSystem->getCPUUsage(&m_Usage);
//}

void SoundManager::UpdateListener(ListenerComponent* listener)
{
	FMOD_VECTOR pos = ToFMOD(listener->GetOwner()->GetPosition());
	FMOD_VECTOR vel{ 0,0,0 };
	FMOD_VECTOR fwd = ToFMOD(listener->GetOwner()->GetWorldDirectionVector(Direction::Forward));
	FMOD_VECTOR up = ToFMOD(listener->GetOwner()->GetWorldDirectionVector(Direction::Up));

	m_CoreSystem->set3DListenerAttributes(0, &pos, &vel, &fwd, &up);
}

void SoundManager::SetVolume_Main(float v)
{
	m_Volume_Main = std::clamp(v, 0.0f, 1.0f);
	m_MainGroup->setVolume(m_Volume_Main);
}

void SoundManager::SetVolume_BGM(float v)
{
	m_Volume_BGM = std::clamp(v, 0.0f, 1.0f);
	m_BGMGroup->setVolume(m_Volume_BGM);
}

void SoundManager::SetVolume_AMB(float v)
{
	m_Volume_AMB = std::clamp(v, 0.0f, 1.0f);
	m_AMBGroup->setVolume(m_Volume_AMB);
}

void SoundManager::SetVolume_SFX(float v)
{
	m_Volume_SFX = std::clamp(v, 0.0f, 1.0f);
	m_SFXGroup->setVolume(m_Volume_SFX);
}

void SoundManager::SetVolume_UI(float v)
{
	m_Volume_UI = std::clamp(v, 0.0f, 1.0f);
	m_UIGroup->setVolume(m_Volume_UI);
}
