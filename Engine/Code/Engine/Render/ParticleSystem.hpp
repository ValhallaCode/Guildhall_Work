#pragma once
#include "Engine/Render/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/BlockAllocator.hpp"
#include <map>
#include <vector>

class SimpleRenderer;
class RenderMesh;
class ParticleEffect;

class Particle
{
public:
	Particle();
	void Update(float deltaSeconds);
	void Render(SimpleRenderer* renderer) const;
	void UpdateRenderMeshAlpha(unsigned char alpha);
	~Particle();
public:
	std::string m_name;
	bool m_isBillboarded;
	float m_timeAlive;
	Vector3 m_velocity;
	Vector3 m_position;
	Vector3 m_acceleration;
	RenderMesh* m_mesh;
};

class ParticleEmitter
{
public:
	ParticleEmitter();
	void Update(float deltaSeconds);
	void Render(SimpleRenderer* renderer) const;
	void BubbleSortParticlesByLifeSpan();
	void SpawnParticles(float deltaSeconds);
	void RemoveDeadParticles();
	~ParticleEmitter();
public:
	std::string m_name;
	Particle* m_defaultParticle;
	std::vector<Particle*> m_particles;
	float m_spawnProbability;
	bool m_initialBurst;
	bool m_isAfffectedByGravity;
	float m_gravity;
	bool m_particleFadeOut;
	float m_particleLifespan;
	Vector3 m_position;
	unsigned int m_count;
	BlockAllocator* m_particleAloc;
	BlockAllocator* m_meshAlloc;
	ParticleEffect* m_parentEffect;
	unsigned int m_createdCount;
	Vector3 m_variance;
};

class ParticleEffect
{
public:
	ParticleEffect();
	void Update(float deltaSeconds);
	void Render(SimpleRenderer* renderer) const;
	void Destroy();
	~ParticleEffect();
public:
	std::string m_name;
	std::vector<ParticleEmitter*> m_emmiters;
	Vector3 m_position;
	bool m_looping;
	bool m_preWarm;
	bool m_destroy;
};

class ParticleSystem
{
public:
	Particle* CreateOrGetParticle(std::string& name, SimpleRenderer* renderer = nullptr, 
		const char* mat_filePath = "", const char* mesh_xml_file_path = "", bool can_billboard = false, 
		Vector3 position = Vector3(0.0f, 0.0f, 0.0f), Vector3 velocity = Vector3(0.0f, 0.0f, 0.0f), 
		Vector3 acceleration = Vector3(0.0f, 0.0f, 0.0f));
	ParticleEmitter* CreateOrGetEmitter(std::string& name, Particle* particle = nullptr, 
		unsigned int count = 1, float spawn_prob = 1.0f, bool particle_fadeout = true, 
		float life_span = 10.0f, Vector3 position = Vector3(0.0f, 0.0f, 0.0f), 
		bool can_burst = false, bool affected_by_gravity = true, float gravity = -9.8f,
		Vector3 variance = Vector3(0.0f, 0.0f, 0.0f));
	ParticleEffect* CreateOrGetEffect(std::string& name, bool can_loop = true, bool is_prewarmed = false, 
		Vector3 position = Vector3(0.0f, 0.0f, 0.0f), int num_emitters = 1, ...);
	ParticleEffect* ParticleSystem::CreateOrGetEffect(std::string& name, bool can_loop = true, 
		bool is_prewarmed = false, Vector3 position = Vector3(0.0f, 0.0f, 0.0f),
		std::vector<std::string> emitter_names = {});
	ParticleEffect* LoadParticleEffectFromFile(const char* file_path, SimpleRenderer* renderer);
	~ParticleSystem();
public:
	std::map<std::string, Particle*> m_particleList;
	std::map<std::string, ParticleEmitter*> m_emmiterList;
	std::map<std::string, ParticleEffect*> m_effectList;
};
