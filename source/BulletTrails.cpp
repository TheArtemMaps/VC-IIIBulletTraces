#include "plugin.h"
#include "Utility.h"
#include "BulletTrails.h"
#include "CCamera.h"
#include "CTxdStore.h"
#include "RenderBuffer.h"
#include "ini.h"
float Thickness;
int Count;
#define ARRAY_SIZE(array)                (sizeof(array) / sizeof(array[0]))
#define FIX_BUGS // Undefine to play with bugs
RwTexture* gpSmokeTrailTexture;

RwIm3DVertex TraceVertices[10];
static RwImVertexIndex TraceIndexList[48] = { 0, 5, 7, 0, 7, 2, 0, 7, 5, 0, 2, 7, 0, 4, 9, 0,
										9, 5, 0, 9, 4, 0, 5, 9, 0, 1, 6, 0, 6, 5, 0, 6,
										1, 0, 5, 6, 0, 3, 8, 0, 8, 5, 0, 8, 3, 0, 5, 8 };


void CBulletTraces::Init(void)
{
	for (int i = 0; i < Count; i++)
		aTraces[i].m_bInUse = false;
	mINI::INIFile file("VCBulletTrails.ini");
	mINI::INIStructure ini;
	file.read(ini);
	//Grand count of traces
	std::string count = ini.get("MAIN").get("Count");
	const char* count2 = count.c_str();
	Count = std::atoi(count2);

	//Thickness
	std::string Thickness2 = ini.get("MAIN").get("Thickness");
	const char* Thickness22 = Thickness2.c_str();
	Thickness = std::atof(Thickness22);

	CTxdStore::PushCurrentTxd();
	int32_t slot2 = CTxdStore::AddTxdSlot("VCBulletTrails");
	CTxdStore::LoadTxd(slot2, GAME_PATH((char*)"MODELS\\VCBULLETTRAILS.TXD"));
	int32_t slot = CTxdStore::FindTxdSlot("VCBulletTrails");
	CTxdStore::SetCurrentTxd(slot);
	gpSmokeTrailTexture = RwTextureRead("smoketrail", NULL);
	CTxdStore::PopCurrentTxd();
	RwIm3DVertexSetU(&TraceVertices[0], 0.0);
	RwIm3DVertexSetV(&TraceVertices[0], 0.0);
	RwIm3DVertexSetU(&TraceVertices[1], 1.0);
	RwIm3DVertexSetV(&TraceVertices[1], 0.0);
	RwIm3DVertexSetU(&TraceVertices[2], 1.0);
	RwIm3DVertexSetV(&TraceVertices[2], 0.0);
	RwIm3DVertexSetU(&TraceVertices[3], 1.0);
	RwIm3DVertexSetV(&TraceVertices[3], 0.0);
	RwIm3DVertexSetU(&TraceVertices[4], 1.0);
	RwIm3DVertexSetV(&TraceVertices[4], 0.0);
	RwIm3DVertexSetU(&TraceVertices[5], 0.0);
	RwIm3DVertexSetU(&TraceVertices[6], 1.0);
	RwIm3DVertexSetU(&TraceVertices[7], 1.0);
	RwIm3DVertexSetU(&TraceVertices[8], 1.0);
	RwIm3DVertexSetU(&TraceVertices[9], 1.0);

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

CBulletTrace CBulletTraces::aTraces[256];

void CBulletTraces::AddTrace(CVector* start, CVector* end, float thickness, uint32_t lifeTime, uint8_t visibility)
{
	int32_t enabledCount;
	uint32_t modifiedLifeTime;
	int32_t nextSlot;

	enabledCount = 0;
	for (int i = 0; i < Count; i++)
		if (aTraces[i].m_bInUse)
			enabledCount++;
	if (enabledCount >= 10)
		modifiedLifeTime = lifeTime / 4;
	else if (enabledCount >= 5)
		modifiedLifeTime = lifeTime / 2;
	else
		modifiedLifeTime = lifeTime;

	nextSlot = 0;
	for (int i = 0; nextSlot < Count && aTraces[i].m_bInUse; i++)
		nextSlot++;
	if (nextSlot < Count) {
		aTraces[nextSlot].m_vecStartPos = *start;
		aTraces[nextSlot].m_vecEndPos = *end;
		aTraces[nextSlot].m_bInUse = true;
		aTraces[nextSlot].m_nCreationTime = CTimer::m_snTimeInMilliseconds;
		aTraces[nextSlot].m_fVisibility = visibility;
		aTraces[nextSlot].m_fThickness = Thickness;
		aTraces[nextSlot].m_nLifeTime = modifiedLifeTime;
	}

	float startProjFwd = DotProduct(TheCamera.GetForward(), *start - TheCamera.GetPosition());
	float endProjFwd = DotProduct(TheCamera.GetForward(), *end - TheCamera.GetPosition());
	if (startProjFwd * endProjFwd < 0.0f) { //if one of point behind us and second before us
		float fStartDistFwd = abs(startProjFwd) / (abs(startProjFwd) + abs(endProjFwd));

		float startProjUp = DotProduct(TheCamera.GetUp(), *start - TheCamera.GetPosition());
		float endProjUp = DotProduct(TheCamera.GetUp(), *end - TheCamera.GetPosition());
		float distUp = (endProjUp - startProjUp) * fStartDistFwd + startProjUp;

		float startProjRight = DotProduct(TheCamera.GetRight(), *start - TheCamera.GetPosition());
		float endProjRight = DotProduct(TheCamera.GetRight(), *end - TheCamera.GetPosition());
		float distRight = (endProjRight - startProjRight) * fStartDistFwd + startProjRight;

		float dist = sqrt(SQR(distUp) + SQR(distRight));
	}
}

void CBulletTraces::AddTrace2(CVector* start, CVector* end, int32_t weaponType, class CEntity* shooter)
{
	CPhysical* player;
	float speed;
	int16_t camMode;

	if (shooter == (CEntity*)FindPlayerPed() || (FindPlayerVehicle(-1, true) != nullptr && FindPlayerVehicle(-1, true) == (CVehicle*)shooter)) {
		camMode = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode;
		if (camMode == MODE_M16_1STPERSON
			|| camMode == MODE_CAMERA
			|| camMode == MODE_SNIPER
			|| camMode == MODE_M16_1STPERSON_RUNABOUT
			|| camMode == MODE_ROCKETLAUNCHER
			|| camMode == MODE_ROCKETLAUNCHER_RUNABOUT
			|| camMode == MODE_SNIPER_RUNABOUT
			|| camMode == MODE_HELICANNON_1STPERSON) {

			player = FindPlayerVehicle(-1, true) ? (CPhysical*)FindPlayerVehicle(-1, true) : (CPhysical*)FindPlayerPed();
			speed = player->m_vecMoveSpeed.Magnitude();
			if (speed < 0.05f)
				return;
		}
	}

	switch (weaponType) {
	case eWeaponType::WEAPON_DESERT_EAGLE:
	case eWeaponType::WEAPON_SHOTGUN:
	case eWeaponType::WEAPON_SPAS12:
	case eWeaponType::WEAPON_SAWNOFF:
		CBulletTraces::AddTrace(start, end, Thickness, 1000, 200);
		break;
	case eWeaponType::WEAPON_M4:
	case eWeaponType::WEAPON_AK47:
	case eWeaponType::WEAPON_SNIPERRIFLE:
	case eWeaponType::WEAPON_MINIGUN:
	case eWeaponType::WEAPON_COUNTRYRIFLE:
	case eWeaponType::WEAPON_MP5:
	case eWeaponType::WEAPON_TEC9:
	case eWeaponType::WEAPON_PISTOL:
	case eWeaponType::WEAPON_MICRO_UZI:
		CBulletTraces::AddTrace(start, end, Thickness, 2000, 220);
		break;
	default:
		CBulletTraces::AddTrace(start, end, Thickness, 750, 150);
		break;
	}
}

CVector operator/(const CVector& vec, float dividend) {
	return { vec.x / dividend, vec.y / dividend, vec.z / dividend };
}

void CBulletTraces::Render(void)
{
	for (int i = 0; i < Count; i++) {
		if (!aTraces[i].m_bInUse)
			continue;
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)(RwTextureGetRaster(gpSmokeTrailTexture)));

		float timeAlive = CTimer::m_snTimeInMilliseconds - aTraces[i].m_nCreationTime;

		float traceThickness = aTraces[i].m_fThickness * timeAlive / aTraces[i].m_nLifeTime;
		CVector horizontalOffset = aTraces[i].m_vecEndPos - aTraces[i].m_vecStartPos;
		horizontalOffset.Normalise();
		horizontalOffset *= traceThickness;

		//then closer trace to die then it more transparent
		uint8_t nAlphaValue = aTraces[i].m_fVisibility * (aTraces[i].m_nLifeTime - timeAlive) / aTraces[i].m_nLifeTime;

		CVector start = aTraces[i].m_vecStartPos;
		CVector end = aTraces[i].m_vecEndPos;
		float startProj = DotProduct(start - TheCamera.GetPosition(), TheCamera.GetForward()) - 0.7f;
		float endProj = DotProduct(end - TheCamera.GetPosition(), TheCamera.GetForward()) - 0.7f;
		if (startProj < 0.0f && endProj < 0.0f) //we dont need render trace behind us
			continue;

		if (startProj < 0.0f) { //if strat behind us move it closer
			float absStartProj = std::abs(startProj);
			float absEndProj = std::abs(endProj);
			start = (absEndProj * start + absStartProj * end) / (absStartProj + absEndProj);
		}
		else if (endProj < 0.0f) {
			float absStartProj = std::abs(startProj);
			float absEndProj = std::abs(endProj);
			end = (absEndProj * start + absStartProj * end) / (absStartProj + absEndProj);
		}

		//we divide trace at three parts
		CVector start2 = (7.0f * start + end) / 8;
		CVector end2 = (7.0f * end + start) / 8;

		RwIm3DVertexSetV(&TraceVertices[5], 10.0f);
		RwIm3DVertexSetV(&TraceVertices[6], 10.0f);
		RwIm3DVertexSetV(&TraceVertices[7], 10.0f);
		RwIm3DVertexSetV(&TraceVertices[8], 10.0f);
		RwIm3DVertexSetV(&TraceVertices[9], 10.0f);

		RwIm3DVertexSetRGBA(&TraceVertices[0], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[1], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[2], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[3], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[4], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[5], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[6], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[7], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[8], 255, 255, 255, nAlphaValue);
		RwIm3DVertexSetRGBA(&TraceVertices[9], 255, 255, 255, nAlphaValue);
		//two points in center
		RwIm3DVertexSetPos(&TraceVertices[0], start2.x, start2.y, start2.z);
		RwIm3DVertexSetPos(&TraceVertices[5], end2.x, end2.y, end2.z);
		//vertical planes
		RwIm3DVertexSetPos(&TraceVertices[1], start2.x, start2.y, start2.z + traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[3], start2.x, start2.y, start2.z - traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[6], end2.x, end2.y, end2.z + traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[8], end2.x, end2.y, end2.z - traceThickness);
		//horizontal planes
		RwIm3DVertexSetPos(&TraceVertices[2], start2.x + horizontalOffset.y, start2.y - horizontalOffset.x, start2.z);
		RwIm3DVertexSetPos(&TraceVertices[7], end2.x + horizontalOffset.y, end2.y - horizontalOffset.x, end2.z);
#ifdef FIX_BUGS //this point calculated wrong for some reason
		RwIm3DVertexSetPos(&TraceVertices[4], start2.x - horizontalOffset.y, start2.y + horizontalOffset.x, start2.z);
		RwIm3DVertexSetPos(&TraceVertices[9], end2.x - horizontalOffset.y, end2.y + horizontalOffset.x, end2.z);
#else
		RwIm3DVertexSetPos(&TraceVertices[4], start2.x - horizontalOffset.y, start2.y - horizontalOffset.y, start2.z);
		RwIm3DVertexSetPos(&TraceVertices[9], end2.x - horizontalOffset.y, end2.y - horizontalOffset.y, end2.z);
#endif

		if (RwIm3DTransform(TraceVertices, ARRAY_SIZE(TraceVertices), nullptr, 1)) {
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TraceIndexList, ARRAY_SIZE(TraceIndexList));
			RwIm3DEnd();
		}

		RwIm3DVertexSetV(&TraceVertices[5], 2.0f);
		RwIm3DVertexSetV(&TraceVertices[6], 2.0f);
		RwIm3DVertexSetV(&TraceVertices[7], 2.0f);
		RwIm3DVertexSetV(&TraceVertices[8], 2.0f);
		RwIm3DVertexSetV(&TraceVertices[9], 2.0f);
		RwIm3DVertexSetRGBA(&TraceVertices[0], 255, 255, 255, 0);
		RwIm3DVertexSetRGBA(&TraceVertices[1], 255, 255, 255, 0);
		RwIm3DVertexSetRGBA(&TraceVertices[2], 255, 255, 255, 0);
		RwIm3DVertexSetRGBA(&TraceVertices[3], 255, 255, 255, 0);
		RwIm3DVertexSetRGBA(&TraceVertices[4], 255, 255, 255, 0);

		RwIm3DVertexSetPos(&TraceVertices[0], start.x, start.y, start.z);
		RwIm3DVertexSetPos(&TraceVertices[1], start.x, start.y, start.z + traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[3], start.x, start.y, start.z - traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[2], start.x + horizontalOffset.y, start.y - horizontalOffset.x, start.z);

		RwIm3DVertexSetPos(&TraceVertices[5], start2.x, start2.y, start2.z);
		RwIm3DVertexSetPos(&TraceVertices[6], start2.x, start2.y, start2.z + traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[8], start2.x, start2.y, start2.z - traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[7], start2.x + horizontalOffset.y, start2.y - horizontalOffset.x, start2.z);
#ifdef FIX_BUGS
		RwIm3DVertexSetPos(&TraceVertices[4], start.x - horizontalOffset.y, start.y + horizontalOffset.x, start.z);
		RwIm3DVertexSetPos(&TraceVertices[9], start2.x - horizontalOffset.y, start2.y + horizontalOffset.x, start2.z);
#else
		RwIm3DVertexSetPos(&TraceVertices[4], start.x - horizontalOffset.y, start.y - horizontalOffset.y, start.z);
		RwIm3DVertexSetPos(&TraceVertices[9], start2.x - horizontalOffset.y, start2.y - horizontalOffset.y, start2.z);
#endif

		if (RwIm3DTransform(TraceVertices, ARRAY_SIZE(TraceVertices), nullptr, rwIM3D_VERTEXUV)) {
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TraceIndexList, ARRAY_SIZE(TraceIndexList));
			RwIm3DEnd();
		}

		RwIm3DVertexSetPos(&TraceVertices[1], end.x, end.y, end.z);
		RwIm3DVertexSetPos(&TraceVertices[2], end.x, end.y, end.z + traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[4], end.x, end.y, end.z - traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[3], end.x + horizontalOffset.y, end.y - horizontalOffset.x, end.z);

		RwIm3DVertexSetPos(&TraceVertices[5], end2.x, end2.y, end2.z);
		RwIm3DVertexSetPos(&TraceVertices[6], end2.x, end2.y, end2.z + traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[8], end2.x, end2.y, end2.z - traceThickness);
		RwIm3DVertexSetPos(&TraceVertices[7], end2.x + horizontalOffset.y, end2.y - horizontalOffset.x, end2.z);
#ifdef FIX_BUGS
		RwIm3DVertexSetPos(&TraceVertices[5], end.x - horizontalOffset.y, end.y + horizontalOffset.x, end.z);
		RwIm3DVertexSetPos(&TraceVertices[9], end2.x - horizontalOffset.y, end2.y + horizontalOffset.x, end2.z);
#else
		RwIm3DVertexSetPos(&TraceVertices[5], end.x - horizontalOffset.y, end.y - horizontalOffset.y, end.z);
		RwIm3DVertexSetPos(&TraceVertices[9], end2.x - horizontalOffset.y, end2.y - horizontalOffset.y, end2.z);
#endif

		if (RwIm3DTransform(TraceVertices, ARRAY_SIZE(TraceVertices), nullptr, rwIM3D_VERTEXUV)) {
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
	for (int i = 0; i < Count; i++) {
		if (aTraces[i].m_bInUse)
			aTraces[i].Update();
	}
}

void CBulletTrace::Update(void)
{
	if (CTimer::m_snTimeInMilliseconds - m_nCreationTime >= m_nLifeTime)
		m_bInUse = false;
}
void CBulletTrace::Shutdown(void) {
	if (gpSmokeTrailTexture) {
		RwTextureDestroy(gpSmokeTrailTexture);
		gpSmokeTrailTexture = nullptr;
	}
}