#include "Emitter.h"

#include "ErrMsg.h"


Emitter::Emitter(const UINT id, const DirectX::BoundingOrientedBox &bounds) : Entity(id, bounds)
{

}


bool Emitter::Initialize(ID3D11Device *device, const std::string &name, const EmitterData &settings, const UINT textureID)
{
	_texID = textureID;

	if (!Entity::Initialize(device, name))
	{
		ErrMsg("Failed to initialize emitter!");
		return false;
	}

	_emitterData = settings;
	if (!_emitterBuffer.Initialize(device, sizeof(EmitterData), &settings))
	{
		ErrMsg("Failed to initialize emitter data buffer!");
		return false;
	}

	Particle *particles = new Particle[settings.particleCount];
	ZeroMemory(particles, sizeof(Particle) * settings.particleCount);

	for (UINT i = 0; i < settings.particleCount; i++)
	{
		/*particles[i].position = {
			(float)((rand() % 2000) - 1000) / 66.66f,
			(float)((rand() % 2000) - 1000) / 66.66f,
			(float)((rand() % 2000) - 1000) / 66.66f
		};

		particles[i].velocity = {
			(float)((rand() % 2000) - 1000) / 3000.0f,
			(float)((rand() % 2000) - 1000) / 3000.0f,
			(float)((rand() % 2000) - 1000) / 3000.0f
		};*/

		particles[i].startPos = {
			(float)((rand() % 2000) - 1000) / 66.66f,
			(float)((rand() % 2000) - 1000) / 66.66f,
			(float)((rand() % 2000) - 1000) / 66.66f,
			0
		};

		/*particles[i].endPos = {
			(float)((rand() % 2000) - 1000) / 66.66f,
			(float)((rand() % 2000) - 1000) / 66.66f,
			(float)((rand() % 2000) - 1000) / 66.66f,
			0
		};*/

		particles[i].endPos = {
			particles[i].startPos.x + (float)((rand() % 2000) - 1000) / 500.0f,
			particles[i].startPos.y + (float)((rand() % 2000) - 1000) / 500.0f,
			particles[i].startPos.z + (float)((rand() % 2000) - 1000) / 500.0f,
			0
		};

		/*particles[i].color = {
			(float)(rand() % 1000) / 1000.0f,
			(float)(rand() % 1000) / 1000.0f,
			(float)(rand() % 1000) / 1000.0f,
			(float)(rand() % 1000) / 1000.0f
		};*/

		particles[i].color = { 1, 1, 1, 1 };

		//particles[i].position.w = 0.05f;
		particles[i].position.w = (float)((rand() % 1000)) / 20000.0f + 0.01f; // size
		particles[i].velocity.w = (float)((rand() % 1000)) / 1000.0f; // lifetime

	}

	if (!_particleBuffer.Initialize(device, sizeof(Particle), 
		settings.particleCount, true, true, false, particles))
	{
		ErrMsg("Failed to initialize emitter particle buffer!");
		delete[] particles;
		return false;
	}

	delete[] particles;
	return true;
}


EntityType Emitter::GetType() const { return EntityType::EMITTER; }


UINT Emitter::GetTextureID() const
{
	return _texID;
}


bool Emitter::Update(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	if (!InternalUpdate(context))
	{
		ErrMsg("Failed to update emitter!");
		return false;
	}

	_emitterData.deltaTime = time.time;
	if (!_emitterBuffer.UpdateBuffer(context, &_emitterData))
	{
		ErrMsg("Failed to update time buffer!");
		return false;
	}

	ID3D11Buffer *emitterBuffer = _emitterBuffer.GetBuffer();
	context->CSSetConstantBuffers(0, 1, &emitterBuffer);

	ID3D11UnorderedAccessView *uav = _particleBuffer.GetUAV();
	context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

	context->Dispatch((UINT)std::ceil(static_cast<float>(_particleBuffer.GetNrOfElements()) / 32.0f), 1, 1);
	
	uav = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

	return true;
}

bool Emitter::BindBuffers(ID3D11DeviceContext *context) const
{
	if (!InternalBindBuffers(context))
	{
		ErrMsg("Failed to bind emitter buffers!");
		return false;
	}

	ID3D11ShaderResourceView *srv = _particleBuffer.GetSRV();
	context->VSSetShaderResources(0, 1, &srv);

	return true;
}

bool Emitter::Render(CameraD3D11 *camera)
{
	if (!InternalRender(camera))
	{
		ErrMsg("Failed to render emitter!");
		return false;
	}

	const ResourceGroup resources = {
		CONTENT_LOAD_ERROR,
		_texID,
	};

	const RenderInstance instance = {
		this,
		sizeof(Emitter)
	};

	camera->QueueEmitter(resources, instance);

	return true;
}


bool Emitter::PerformDrawCall(ID3D11DeviceContext* context) const
{
	context->Draw(static_cast<UINT>(_emitterData.particleCount), 0);
	return true;
}
