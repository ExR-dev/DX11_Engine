#include "Emitter.h"

#include "ErrMsg.h"


Emitter::Emitter(const UINT id, const DirectX::BoundingBox &bounds) : Entity(id, bounds)
{

}

bool Emitter::Initialize(ID3D11Device *device, const EmitterData *settings)
{
	if (!Entity::Initialize(device))
	{
		ErrMsg("Failed to initialize emitter!");
		return false;
	}

	if (settings == nullptr)
	{
		
	}

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

	return true;
}

bool Emitter::BindBuffers(ID3D11DeviceContext *context) const
{
	/*if (!InternalBindBuffers(context))
	{
		ErrMsg("Failed to bind emitter buffers!");
		return false;
	}*/

	/*ID3D11Buffer *const wmBuffer = _transform.GetConstantBuffer();
	context->GSSetConstantBuffers(0, 1, &wmBuffer);*/

	return true;
}

bool Emitter::Render(CameraD3D11 *camera)
{
	if (!InternalRender(camera))
	{
		ErrMsg("Failed to render emitter!");
		return false;
	}

	return true;
}


bool Emitter::UpdateParticles(ID3D11DeviceContext *context) const
{
	/*ID3D11UnorderedAccessView *uav = _particleBuffer.GetUAV();
	context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
	context->Dispatch(std::ceil(static_cast<float>(_particleBuffer.GetNrOfElements()) / 32.0f), 1, 1);

	uav = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);*/
	return true;
}

bool Emitter::PerformDrawCall(ID3D11DeviceContext* context) const
{
	//context->DrawIndexed(static_cast<UINT>(_nrOfIndices), static_cast<UINT>(_startIndex), 0);
	return true;
}
