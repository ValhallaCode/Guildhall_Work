#include "Engine/Render/ParticleSystem.hpp"
#include "Engine/RHI/RenderMesh.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <stdarg.h>

#define EVER (;;)


Particle* ParticleSystem::CreateOrGetParticle(std::string& name, SimpleRenderer* renderer /*= nullptr*/, 
	const char* mat_filePath /*= ""*/, const char* mesh_xml_file_path /*= ""*/, bool can_billboard /*= false*/, 
	Vector3 position /*= Vector3(0.0f, 0.0f, 0.0f)*/, Vector3 velocity /*= Vector3(0.0f, 0.0f, 0.0f)*/, 
	Vector3 acceleration /*= Vector3(0.0f, 0.0f, 0.0f)*/)
{
	for (auto iterate : m_particleList)
	{
		if (iterate.first == name)
			return iterate.second;
	}

	Particle* particle = new Particle();
	Mesh* mesh = CreateOrGetMesh(name, mesh_xml_file_path);
	Material* material = CreateOrGetMaterial(name, renderer, mat_filePath);
	particle->m_mesh = new RenderMesh();
	particle->m_mesh->InsertMesh(name, *mesh);
	particle->m_mesh->InsertMaterial(name, *material);

	particle->m_name = name;
	particle->m_position = position;
	particle->m_velocity = velocity;
	particle->m_timeAlive = 0.0f;
	particle->m_acceleration = acceleration;
	particle->m_isBillboarded = can_billboard;

	m_particleList.insert_or_assign(name, particle);
	return particle;
}

ParticleEmitter* ParticleSystem::CreateOrGetEmitter(std::string& name, Particle* particle /*= nullptr*/,
	unsigned int count /*= 1*/, float spawn_prob /*= 1.0f*/, bool particle_fadeout /*= true*/, 
	float life_span /*= 10.0f*/, Vector3 position /*= Vector3(0.0f, 0.0f, 0.0f)*/, 
	bool can_burst /*= false*/, bool affected_by_gravity /*= true*/, float gravity /*= -9.8f*/,
	Vector3 variance /*= Vector3(0.0f, 0.0f, 0.0f)*/)
{
	for (auto iterate : m_emmiterList)
	{
		if (iterate.first == name)
			return iterate.second;
	}

	ASSERT_OR_DIE(particle != nullptr, "Tried to create an emitter with no particle!");

	ParticleEmitter* emitter = new ParticleEmitter();
	emitter->m_name = name;
	emitter->m_defaultParticle = particle;
	emitter->m_count = count;
	emitter->m_spawnProbability = spawn_prob;
	emitter->m_particleFadeOut = particle_fadeout;
	emitter->m_particleLifespan = life_span;
	emitter->m_position = position;
	emitter->m_initialBurst = can_burst;
	emitter->m_isAfffectedByGravity = affected_by_gravity;
	emitter->m_gravity = gravity;
	emitter->m_particles.reserve(count);
	emitter->m_particleAloc = new BlockAllocator(sizeof(Particle) * count);
	emitter->m_createdCount = 0;
	emitter->m_variance = variance;
	emitter->m_meshAlloc = new BlockAllocator(sizeof(RenderMesh) * count);

	m_emmiterList.insert_or_assign(name, emitter);
	return emitter;
}

ParticleEffect* ParticleSystem::CreateOrGetEffect(std::string& name, bool can_loop /*= true*/, 
	bool is_prewarmed /*= false*/, Vector3 position /*= Vector3(0.0f, 0.0f, 0.0f)*/, 
	int num_emitters /*= 1*/, ...)
{
	for (auto iterate : m_effectList)
	{
		if (iterate.first == name)
			return iterate.second;
	}

	ParticleEffect* effect = new ParticleEffect();
	effect->m_name = name;
	effect->m_looping = can_loop;
	effect->m_preWarm = is_prewarmed;

	va_list params;
	va_start(params, num_emitters);

	effect->m_emmiters.resize(num_emitters);
	for (int index = 0; index < num_emitters; ++index)
	{
		const char* emitter_name = va_arg(params, const char*);
		ASSERT_OR_DIE(emitter_name != nullptr, "Trying to added Emitter to Effect " + name + " without Emitter name!");
		// If Assert in create, then Emitter was not created beforehand!
		std::string emit_name = emitter_name;
		effect->m_emmiters[index] = CreateOrGetEmitter(emit_name);
		effect->m_emmiters[index]->m_position += position;
		effect->m_emmiters[index]->m_parentEffect = effect;
	}
	va_end(params);

	m_effectList.insert_or_assign(name, effect);
	return effect;
}

ParticleEffect* ParticleSystem::CreateOrGetEffect(std::string& name, bool can_loop /*= true*/, 
	bool is_prewarmed /*= false*/, Vector3 position /*= Vector3(0.0f, 0.0f, 0.0f)*/,
	std::vector<std::string> emitter_names /*= {}*/)
{
	for (auto iterate : m_effectList)
	{
		if (iterate.first == name)
			return iterate.second;
	}

	ParticleEffect* effect = new ParticleEffect();
	effect->m_name = name;
	effect->m_looping = can_loop;
	effect->m_preWarm = is_prewarmed;

	ASSERT_OR_DIE(!emitter_names.empty(), "Tried to create Effect without Emitter!");

	effect->m_emmiters.resize(emitter_names.size());
	for (unsigned int index = 0; index < emitter_names.size(); ++index)
	{
		// If Assert in create, then Emitter was not created beforehand!
		std::string emit_name = emitter_names[index];
		effect->m_emmiters[index] = CreateOrGetEmitter(emit_name);
		effect->m_emmiters[index]->m_position += position;
		effect->m_emmiters[index]->m_parentEffect = effect;
	}

	m_effectList.insert_or_assign(name, effect);
	return effect;
}

ParticleEffect* ParticleSystem::LoadParticleEffectFromFile(const char* file_path, SimpleRenderer* renderer)
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile(file_path);
	if (result != tinyxml2::XML_SUCCESS)
	{
		return nullptr;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return nullptr;
	}

	std::vector<std::string> emitter_names;
	for (auto emitElem = xmlRoot->FirstChildElement("Emitter"); emitElem != nullptr; emitElem = emitElem->NextSiblingElement("Emitter"))
	{
		auto partElem = emitElem->FirstChildElement("Particle");
		const char* part_name = ParseXmlAttribute(*partElem, "name", "DEFAULT!");

		auto matElem = partElem->FirstChildElement("Material");
		const char* mat_file_path = ParseXmlAttribute(*matElem, "xml_path", "FILE_NOT_GIVEN!");

		auto meshElem = partElem->FirstChildElement("Mesh");
		const char* mesh_file_path = ParseXmlAttribute(*meshElem, "xml_path", "FILE_NOT_GIVEN!");

		auto physElem = partElem->FirstChildElement("Physics");
		auto posElem = physElem->FirstChildElement("Offset");
		Vector3 part_pos = ParseXmlAttribute(*posElem, "val", Vector3(0.0f, 0.0f, 0.0f));

		auto velElem = physElem->FirstChildElement("Velocity");
		Vector3 part_vel = ParseXmlAttribute(*velElem, "val", Vector3(0.0f, 0.0f, 0.0f));

		auto accelElem = physElem->FirstChildElement("Acceleration");
		Vector3 part_accel = ParseXmlAttribute(*accelElem, "val", Vector3(0.0f, 0.0f, 0.0f));

		auto billElem = partElem->FirstChildElement("Billboarding");
		bool can_billboard = false;
		if (billElem != nullptr)
			can_billboard = true;

		std::string part_string = part_name;
		Particle* particle = CreateOrGetParticle(part_string, renderer, mat_file_path, mesh_file_path, can_billboard, part_pos, part_vel, part_accel);

		const char* emitter_name = ParseXmlAttribute(*emitElem, "name", "DEFAULT!");
		std::string emit_string = emitter_name;
		emitter_names.push_back(emit_string);

		auto spawnElem = emitElem->FirstChildElement("Spawning");
		auto countElem = spawnElem->FirstChildElement("Count");
		unsigned int part_count = ParseXmlAttribute(*countElem, "val", 100);

		auto probElem = spawnElem->FirstChildElement("Probability");
		float spawn_prob = ParseXmlAttribute(*probElem, "val", 1.0f);
		spawn_prob = ClampNormalizedFloat(spawn_prob);

		bool can_fade = false;
		auto fadeElem = spawnElem->FirstChildElement("Fade_Out");
		if (fadeElem != nullptr)
			can_fade = true;

		auto lifeElem = spawnElem->FirstChildElement("Life_Span");
		float life_span = ParseXmlAttribute(*lifeElem, "val", 10.0f);

		auto positionElem = spawnElem->FirstChildElement("Offset");
		Vector3 emit_pos = ParseXmlAttribute(*positionElem, "val", Vector3(0.0f, 0.0f, 0.0f));

		auto burstElem = spawnElem->FirstChildElement("Burst");
		bool can_burst = false;
		if (burstElem != nullptr)
		{
			can_burst = true;
		}

		auto gravElem = spawnElem->FirstChildElement("Gravity");
		bool gravity_affect = false;
		float gravity = 0.0f;
		if (gravElem != nullptr)
		{
			gravity_affect = true;
			gravity = ParseXmlAttribute(*gravElem, "val", 0.0f);
		}

		auto varElem = spawnElem->FirstChildElement("Variance");
		Vector3 variance = ParseXmlAttribute(*varElem, "val", Vector3(0.0f, 0.0f, 0.0f));

		CreateOrGetEmitter(emit_string, particle, part_count, spawn_prob, can_fade, life_span, emit_pos, can_burst, gravity_affect, gravity, variance);
	}

	const char* effect_name = ParseXmlAttribute(*xmlRoot, "name", "DEFAULT!");

	auto loopElem = xmlRoot->FirstChildElement("Looping");
	bool can_loop = false;
	if (loopElem != nullptr)
		can_loop = true;

	auto warmElem = xmlRoot->FirstChildElement("Prewarm");
	bool is_prewarmed = false;
	if (warmElem != nullptr)
		is_prewarmed = true;

	auto initPos = xmlRoot->FirstChildElement("Offset");
	Vector3 effect_pos = ParseXmlAttribute(*initPos, "val", Vector3(0.0f, 0.0f, 0.0f));

	std::string effect_string = effect_name;
	return CreateOrGetEffect(effect_string, can_loop, is_prewarmed, effect_pos, emitter_names);
}

ParticleSystem::~ParticleSystem()
{
	m_effectList.clear();
	m_emmiterList.clear();
	m_particleList.clear();
}

Particle::Particle()
	:m_timeAlive(0.0f)
	, m_mesh(nullptr)
{

}

void Particle::Update(float deltaSeconds)
{
	m_timeAlive += deltaSeconds;

	m_velocity += deltaSeconds * m_acceleration;
	m_position += deltaSeconds * m_velocity;
}

void Particle::Render(SimpleRenderer* renderer) const
{
	
	if (m_isBillboarded)
	{
		Matrix4 transform;
		transform.SetTranslate(m_position);
		renderer->SetModelMatrix(transform);

		Matrix4 camera = renderer->m_matrixData.view;
		Vector3 right = camera.GetIBasis();
		Vector3 up = camera.GetJBasis();
		Mesh* mesh = &m_mesh->m_meshs.begin()->second;
		mesh->m_vertices.clear();
		mesh->m_indices.clear();
		if (mesh->m_type == "One_Sided_Quad")
		{
			Vector3 new_pos = Vector3(0.0f, 0.0f, 0.0f);
			mesh->CreateOneSidedQuadWithBillboard(new_pos, mesh->m_extension, right, up, mesh->m_color);
		}
		else
		{
			Vector3 new_pos = Vector3(0.0f, 0.0f, 0.0f);
			mesh->CreateTwoSidedQuadWithBillboard(new_pos, mesh->m_extension, right, up, mesh->m_color);
		}
	}
	else
	{
		Matrix4 transform;
		transform.SetTranslate(m_position);
		renderer->SetModelMatrix(transform);
	}

	renderer->DrawRenderMesh(*m_mesh);
	renderer->MakeModelMatrixIdentity();
}

void Particle::UpdateRenderMeshAlpha(unsigned char alpha)
{
	Mesh* mesh = &m_mesh->m_meshs.begin()->second;
	for (unsigned int iterate = 0; iterate < mesh->m_vertices.size(); ++iterate)
	{
		Vertex3_PCT* vertex = &mesh->m_vertices[iterate];
		vertex->m_color.a = alpha;
	}
}

Particle::~Particle()
{
	//delete m_mesh;
}

ParticleEmitter::ParticleEmitter()
	:m_defaultParticle(nullptr)
	, m_particleAloc(nullptr)
	, m_parentEffect(nullptr)
{
	
}

void ParticleEmitter::Update(float deltaSeconds)
{
	if(!m_parentEffect->m_destroy)
		SpawnParticles(deltaSeconds);
	BubbleSortParticlesByLifeSpan();
	RemoveDeadParticles();
	
	for (auto iterate : m_particles)
	{
		if (m_isAfffectedByGravity)
			iterate->m_acceleration += Vector3(0.0f, m_gravity, 0.0f);

		if (m_particleFadeOut)
		{
			float map_val = m_particleLifespan - iterate->m_timeAlive;

			float alpha_factor = RangeMapFloat(0.0f, m_particleLifespan, 0.0f, 1.0f, map_val);
			float orig_alpha = (float)m_defaultParticle->m_mesh->m_meshs.begin()->second.m_vertices[0].m_color.a;

			unsigned char new_alpha = (unsigned char)floor(orig_alpha * alpha_factor);
			iterate->UpdateRenderMeshAlpha(new_alpha);
		}

		iterate->Update(deltaSeconds);
	}
}

void ParticleEmitter::Render(SimpleRenderer* renderer) const
{
	Matrix4 transform;
	transform.SetTranslate(m_position);

	renderer->SetModelMatrix(transform);

	for (auto particle : m_particles) 
	{
		

		particle->Render(renderer);
	}
}

void ParticleEmitter::BubbleSortParticlesByLifeSpan()
{
	bool did_swap = false;
	for EVER 
	{
		did_swap = false;
		for (size_t i = 0; i < m_particles.size() - 1; i++) 
		{
			if (m_particles[i]->m_timeAlive > m_particles[i + 1]->m_timeAlive)
			{
				Particle* temp = m_particles[i];
				m_particles[i] = m_particles[i + 1];
				m_particles[i + 1] = temp;
				did_swap = true;
			}
		}

		if (!did_swap)
			break;
	}
}

void ParticleEmitter::SpawnParticles(float deltaSeconds)
{
	if (m_particles.size() < m_count)
	{
		if (!m_parentEffect->m_looping && m_count == m_createdCount)
			return;

		float xVar = GetRandomFloatInRange(-m_variance.x, m_variance.x);
		float yVar = GetRandomFloatInRange(-m_variance.y, m_variance.y);
		float zVar = GetRandomFloatInRange(-m_variance.z, m_variance.z);

		if(m_initialBurst)
		{
			for (unsigned int num_to_spawn = 0; num_to_spawn < m_count - m_particles.size(); ++num_to_spawn)
			{
				float chance = GetRandomFloatInRange(0.0f, 1.0f);
				if (chance <= m_spawnProbability)
				{
					++m_createdCount;
					Particle* particle = m_particleAloc->create<Particle>();
					particle->m_name = m_defaultParticle->m_name;
					particle->m_isBillboarded = m_defaultParticle->m_isBillboarded;
					particle->m_timeAlive = m_defaultParticle->m_timeAlive;
					particle->m_position += m_position + m_defaultParticle->m_position;

					particle->m_mesh = m_meshAlloc->create<RenderMesh>();
					particle->m_mesh->m_materials = m_defaultParticle->m_mesh->m_materials;
					particle->m_mesh->m_meshs = m_defaultParticle->m_mesh->m_meshs;

					particle->m_acceleration += Vector3(xVar, yVar, zVar);
					m_particles.push_back(particle);
				}
			}
		}
		else
		{
			uint total_frames = (uint)(m_particleLifespan / deltaSeconds);
			uint particles_remaining = m_count - m_particles.size();
			uint particles_to_spawn = (uint)ceil((float)particles_remaining / (float)total_frames);

			for(uint count = 0; count <= particles_to_spawn; ++count)
			{
				float chance = GetRandomFloatInRange(0.0f, 1.0f);
				if (chance <= m_spawnProbability)
				{
					++m_createdCount;
					Particle* particle = m_particleAloc->create<Particle>(); 
					particle->m_name = m_defaultParticle->m_name;
					particle->m_isBillboarded = m_defaultParticle->m_isBillboarded;
					particle->m_timeAlive = m_defaultParticle->m_timeAlive;
					particle->m_position += m_position + m_defaultParticle->m_position;

					particle->m_mesh = m_meshAlloc->create<RenderMesh>();
					particle->m_mesh->m_materials = m_defaultParticle->m_mesh->m_materials;
					particle->m_mesh->m_meshs = m_defaultParticle->m_mesh->m_meshs;

					particle->m_acceleration += Vector3(xVar, yVar, zVar);
					m_particles.push_back(particle);
				}
			}
		}
	}
}

void ParticleEmitter::RemoveDeadParticles()
{
	// Sorted so that oldest is always in the back
	// so we can just pop the back until we reach the life condition
	for (uint index = m_particles.size() - 1; index >= 0; --index)
	{
		if (m_particles[index]->m_timeAlive >= m_particleLifespan)
		{
			m_particleAloc->free(m_particles[index]);
			m_meshAlloc->free(m_particles[index]->m_mesh);
			m_particles.pop_back();
		}
		else
			break;
	}
}

ParticleEmitter::~ParticleEmitter()
{
	delete m_defaultParticle;

	m_particles.clear();

	delete m_particleAloc;
}

ParticleEffect::ParticleEffect()
	:m_destroy(false)
{

}

void ParticleEffect::Update(float deltaSeconds)
{
	if (m_preWarm)
	{
		for (auto iterate : m_emmiters)
		{
			for (float total_time = 0.0f; iterate->m_particleLifespan > total_time; total_time += deltaSeconds) 
			{
				iterate->Update(deltaSeconds);
			}
		}
		m_preWarm = false;
	}
	else
	{
		for (auto iterate : m_emmiters)
		{
			iterate->Update(deltaSeconds);
		}
	}
}

void ParticleEffect::Render(SimpleRenderer* renderer) const
{
	for (auto iterate : m_emmiters)
	{
		iterate->Render(renderer);
	}
}

void ParticleEffect::Destroy()
{
	m_destroy = true;
	for (auto iterate : m_emmiters)
	{
		if (!iterate->m_particles.empty())
			return;
	}
	this->~ParticleEffect();
}

ParticleEffect::~ParticleEffect()
{
	m_emmiters.clear();
}

/*
Vector3 m_position;
*/