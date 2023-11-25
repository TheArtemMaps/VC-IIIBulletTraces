#include "plugin.h"

#include "BulletTrails.h"
using namespace plugin;
plugin::ThiscallEvent<AddressList<0x53E175, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> movingThingsEvent;
class VCBulletTraces {
public:
    VCBulletTraces() {
        plugin::Events::initRwEvent += []() {
            CBulletTraces::Init();
        };
        movingThingsEvent += []() {
            CBulletTraces::Render();
        };
        plugin::Events::gameProcessEvent += []() {
            CBulletTraces::Update();
        };
       // plugin::Events::shutdownRwEvent += []() {
       //     CBulletTraces::Shutdown();
       // };
        //patch::RedirectJump(0x726AF0, CBulletTraces::AddTrace2);
        patch::RedirectJump(0x723750, CBulletTraces::AddTrace);
        //patch::RedirectJump(0x723C10, CBulletTraces::Render);
        //patch::RedirectJump(0x723FB0, CBulletTraces::Update);
        //patch::RedirectJump(0x721D50, CBulletTraces::Init);
    }
} _VCBulletTraces;
