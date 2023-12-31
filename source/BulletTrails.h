#pragma once

struct CBulletTrace
{
	CVector m_vecStartPos;
	CVector m_vecEndPos;
	bool m_bInUse;
	uint32_t m_nCreationTime;
	uint32_t m_nLifeTime;
	float m_fThickness;
	uint8_t m_fVisibility;

	void Update(void);
};


class CBulletTraces
{
public:
	static CBulletTrace aTraces[255];

	static void Init(void);
	static void Render(void);
	static void Update(void);
	static void Shutdown(void);
	static void AddTrace(CVector* start, CVector* end, float thickness, uint32_t lifeTime, uint8_t visibility);
	static void AddTrace2(CVector* start, CVector* end, int32_t weaponType, class CEntity* shooter);

public:
	static float thickness[512];
	static int lifetime[512];
	static int visibility[512];

};
