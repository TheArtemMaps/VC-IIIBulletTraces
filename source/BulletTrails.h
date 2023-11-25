#pragma once
struct CBulletTrace
{
	CVector m_vecCurrentPos;
	CVector m_vecTargetPos;
	bool m_bInUse;
	uint8_t m_framesInUse;
	uint8_t m_lifeTime;

	void Update(void);
};

class CBulletTraces
{
public:
	static CBulletTrace aTraces[16];

	static void Init(void);
	static void AddTrace(CVector*, CVector*);
	static void Render(void);
	static void Update(void);
};