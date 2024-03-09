#include "Emitter.h"

#include "ErrMsg.h"


Emitter::Emitter(const UINT id, const DirectX::BoundingBox &bounds) : Entity(id, bounds)
{

}


bool Emitter::Initialize(ID3D11Device *device, const EmitterData &settings)
{
	if (!Entity::Initialize(device))
	{
		ErrMsg("Failed to initialize emitter!");
		return false;
	}

	_settings = settings;
	if (!_emitterBuffer.Initialize(device, sizeof(EmitterData), &settings))
	{
		ErrMsg("Failed to initialize emitter data buffer!");
		return false;
	}

	constexpr float time[4] = { 1.0f / 60.0f, 0.0f , 0, 0 };
	if (!_timeBuffer.Initialize(device, sizeof(float) * 4, &time))
	{
		ErrMsg("Failed to initialize emitter time buffer!");
		return false;
	}

	Particle *particles = new Particle[settings.particleCount];
	ZeroMemory(particles, sizeof(Particle) * settings.particleCount);

	for (int i = 0; i < settings.particleCount; i++)
	{
		particles[i].position = {
			(float)((rand() % 2000) - 1000) / 250.0f,
			(float)((rand() % 2000) - 1000) / 250.0f,
			(float)((rand() % 2000) - 1000) / 250.0f
		};

		//particles[i].velocity = {
		//	(float)((rand() % 2000) - 1000) / 1000.0f,
		//	(float)((rand() % 2000) - 1000) / 1000.0f,
		//	(float)((rand() % 2000) - 1000) / 1000.0f
		//};

		//particles[i].color = {
		//	(float)(rand() % 1000) / 1000.0f,
		//	(float)(rand() % 1000) / 1000.0f,
		//	(float)(rand() % 1000) / 1000.0f
		//};

		particles[i].size = (float)((rand() % 1000)) / 20000.0f + 0.02f;
		particles[i].lifetime = (float)((rand() % 1000)) / 1000.0f;

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


bool Emitter::Update(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	if (!InternalUpdate(context))
	{
		ErrMsg("Failed to update emitter!");
		return false;
	}

	const float timeData[4] = { time.time, time.deltaTime, 0, 0 };
	if (!_timeBuffer.UpdateBuffer(context, &timeData))
	{
		ErrMsg("Failed to update time buffer!");
		return false;
	}

	ID3D11Buffer *buf = _emitterBuffer.GetBuffer();
	context->CSSetConstantBuffers(0, 1, &buf);

	buf = _timeBuffer.GetBuffer();
	context->CSSetConstantBuffers(1, 1, &buf);

	ID3D11UnorderedAccessView *uav = _particleBuffer.GetUAV();
	context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

	context->Dispatch(std::ceil(static_cast<float>(_particleBuffer.GetNrOfElements()) / 32.0f), 1, 1);

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

	camera->QueueEmitter({ this, sizeof(Emitter) });

	return true;
}


bool Emitter::PerformDrawCall(ID3D11DeviceContext* context) const
{
	context->Draw(static_cast<UINT>(_settings.particleCount), 0);
	return true;
}
