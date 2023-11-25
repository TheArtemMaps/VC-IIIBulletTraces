#include "plugin.h"
#include "Utility.h"
#include "BulletTrails.h"
#include "CCamera.h"
#include "CTxdStore.h"
#include "RenderBuffer.h"
#include "ini.h"
#include "CShadows.h"
#include <format>
#define ARRAY_SIZE(array)                (sizeof(array) / sizeof(array[0]))
#define FIX_BUGS // Undefine to play with bugs
RwTexture* gpTraceTexture;

CVector
CrossProduct(const CVector& v1, const CVector& v2)
{
	return CVector(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

RwIm3DVertex TraceVertices[6];
RwImVertexIndex TraceIndexList[12];

CVector operator/(const CVector& vec, float dividend) {
	return { vec.x / dividend, vec.y / dividend, vec.z / dividend };
}

CBulletTrace CBulletTraces::aTraces[16];

void CBulletTraces::Init(void)
{
	for (int i = 0; i < 16; i++)
		aTraces[i].m_bInUse = false;
	CTxdStore::PushCurrentTxd();
	int32_t slot2 = CTxdStore::AddTxdSlot("IIIBulletTraces");
	CTxdStore::LoadTxd(slot2, PLUGIN_PATH((char*)"MODELS\\IIIBULLETTRACES.TXD"));
	int32_t slot = CTxdStore::FindTxdSlot("IIIBulletTraces");
	CTxdStore::SetCurrentTxd(slot);
	gpTraceTexture = RwTextureRead("trace", NULL);
	RwIm3DVertexSetRGBA(&TraceVertices[0], 20, 20, 20, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[1], 20, 20, 20, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[2], 70, 70, 70, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[3], 70, 70, 70, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[4], 10, 10, 10, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[5], 10, 10, 10, 255);
	RwIm3DVertexSetU(&TraceVertices[0], 0.0);
	RwIm3DVertexSetV(&TraceVertices[0], 0.0);
	RwIm3DVertexSetU(&TraceVertices[1], 1.0);
	RwIm3DVertexSetV(&TraceVertices[1], 0.0);
	RwIm3DVertexSetU(&TraceVertices[2], 0.0);
	RwIm3DVertexSetV(&TraceVertices[2], 0.5);
	RwIm3DVertexSetU(&TraceVertices[3], 1.0);
	RwIm3DVertexSetV(&TraceVertices[3], 0.5);
	RwIm3DVertexSetU(&TraceVertices[4], 0.0);
	RwIm3DVertexSetV(&TraceVertices[4], 1.0);
	RwIm3DVertexSetU(&TraceVertices[5], 1.0);
	RwIm3DVertexSetV(&TraceVertices[5], 1.0);

	TraceIndexList[0] = 0;
	TraceIndexList[1] = 2;
	TraceIndexList[2] = 1;
	TraceIndexList[3] = 1;
	TraceIndexList[4] = 2;
	TraceIndexList[5] = 3;
	TraceIndexList[6] = 2;
	TraceIndexList[7] = 4;
	TraceIndexList[8] = 3;
	TraceIndexList[9] = 3;
	TraceIndexList[10] = 4;
	TraceIndexList[11] = 5;
}

void CBulletTraces::AddTrace(CVector* vecStart, CVector* vecTarget)
{
	int index;
	for (index = 0; index < 16; index++) {
		if (!aTraces[index].m_bInUse)
			break;
	}
	if (index == 16)
		return;
	aTraces[index].m_vecCurrentPos = *vecStart;
	aTraces[index].m_vecTargetPos = *vecTarget;
	aTraces[index].m_bInUse = true;
	aTraces[index].m_framesInUse = 0;
	aTraces[index].m_lifeTime = 25 + GetRandomNumber() % 32;
}

void CBulletTraces::Render(void)
{
	for (int i = 0; i < 16; i++) {
		if (!aTraces[i].m_bInUse)
			continue;
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
#ifdef FIX_BUGS
		// Raster has no transparent pixels so it relies on the raster format having alpha
		// to turn on blending. librw image conversion might get rid of it right now so let's
		// just force it on.
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
#endif
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpTraceTexture));
		CVector inf = aTraces[i].m_vecCurrentPos;
		CVector sup = aTraces[i].m_vecTargetPos;
		CVector center = (inf + sup) / 2;
		CVector width = CrossProduct(TheCamera.GetForward(), (sup - inf));
		width.Normalise();
		width /= 20;
		uint8_t intensity = aTraces[i].m_lifeTime;
		for (int i = 0; i < ARRAY_SIZE(TraceVertices); i++)
			RwIm3DVertexSetRGBA(&TraceVertices[i], intensity, intensity, intensity, 0xFF);
		RwIm3DVertexSetPos(&TraceVertices[0], inf.x + width.x, inf.y + width.y, inf.z + width.z);
		RwIm3DVertexSetPos(&TraceVertices[1], inf.x - width.x, inf.y - width.y, inf.z - width.z);
		RwIm3DVertexSetPos(&TraceVertices[2], center.x + width.x, center.y + width.y, center.z + width.z);
		RwIm3DVertexSetPos(&TraceVertices[3], center.x - width.x, center.y - width.y, center.z - width.z);
		RwIm3DVertexSetPos(&TraceVertices[4], sup.x + width.x, sup.y + width.y, sup.z + width.z);
		RwIm3DVertexSetPos(&TraceVertices[5], sup.x - width.x, sup.y - width.y, sup.z - width.z);
		if (RwIm3DTransform(TraceVertices, ARRAY_SIZE(TraceVertices), NULL, rwIM3D_VERTEXUV)) {
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TraceIndexList, ARRAY_SIZE(TraceIndexList));
			RwIm3DEnd();
		}
	}
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}

void CBulletTraces::Update(void)
{
	for (int i = 0; i < 16; i++) {
		if (aTraces[i].m_bInUse)
			aTraces[i].Update();
	}
}

void CBulletTrace::Update(void)
{
	if (m_framesInUse == 0) {
		m_framesInUse++;
		return;
	}
	if (m_framesInUse > 60) {
		m_bInUse = false;
		return;
	}
	CVector diff = m_vecCurrentPos - m_vecTargetPos;
	float remaining = diff.Magnitude();
	if (remaining > 0.8f)
		m_vecCurrentPos = m_vecTargetPos + (remaining - 0.8f) / remaining * diff;
	else
		m_bInUse = false;
	if (--m_lifeTime == 0)
		m_bInUse = false;
	m_framesInUse++;
}