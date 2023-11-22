#include "plugin.h"

#include "BulletTrails.h"
using namespace plugin;
plugin::CdeclEvent <plugin::AddressList<0x726AD0, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickNone, void()> movingThingsEvent;
class VCBulletTraces {
public:
    VCBulletTraces() {
        plugin::Events::initGameEvent += []() {
            CBulletTraces::Init();
        };
        movingThingsEvent += []() {
            CBulletTraces::Render();
        };
        plugin::Events::gameProcessEvent += []() {
            CBulletTraces::Update();
        };
        plugin::Events::shutdownRwEvent += []() {
            CBulletTrace::Shutdown();
        };
        patch::RedirectJump(0x726AF0, CBulletTraces::AddTrace2);
        patch::RedirectJump(0x723750, CBulletTraces::AddTrace);
        //patch::RedirectJump(0x723C10, CBulletTraces::Render);
        //patch::RedirectJump(0x723FB0, CBulletTraces::Update);
        //patch::RedirectJump(0x721D50, CBulletTraces::Init);
    }
} _VCBulletTraces;
