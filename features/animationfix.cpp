#include "animationfix.h"

void animfix::build_server_bones(IBasePlayer* player)
{
    const auto backup_occlusion_flags = player->GetOcclusionFlags();
    const auto backup_occlusion_framecount = player->GetOcclusionFramecount();

    player->GetOcclusionFlags() = 0;
    player->GetOcclusionFramecount() = 0;

    player->GetReadableBones() = player->GetWritableBones() = 0;

    player->InvalidateBoneCache();

    player->GetEffects() |= 0x8;

    const auto backup_bone_array = player->GetBoneArrayForWrite();
    player->GetBoneArrayForWrite() = bones;

    csgo->UpdateMatrix = true;
    player->SetupBones(nullptr, -1, 0x7FF00, player->GetSimulationTime());
    csgo->UpdateMatrix = false;

    player->GetBoneArrayForWrite() = backup_bone_array;
    player->GetOcclusionFlags() = backup_occlusion_flags;
    player->GetOcclusionFramecount() = backup_occlusion_framecount;

    player->GetEffects() &= ~0x8;
}

void animfix::update_player(IBasePlayer* player, animation* record, animation* previous)
{
    static auto& enable_bone_cache_invalidation = **reinterpret_cast<bool**>(
        reinterpret_cast<uint32_t>((void*)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
            hs::bone_cache_validation::s().c_str())) + 2);

    const auto backup_frametime = interfaces.global_vars->frametime;
    const auto backup_curtime = interfaces.global_vars->curtime;
    const auto old_flags = player->GetFlagsPtr();

    auto state = player->GetPlayerAnimState();

    if (state->m_iLastClientSideAnimationUpdateFramecount == interfaces.global_vars->framecount)
        state->m_iLastClientSideAnimationUpdateFramecount -= 1.f;

    interfaces.global_vars->frametime = interfaces.global_vars->interval_per_tick;
    interfaces.global_vars->curtime = player->GetSimulationTime();

    if (player->GetAnimOverlay(5)->m_flWeight > 0.0f)
        player->GetFlagsPtr() |= FL_ONGROUND;

    player->GetEFlags() &= ~0x1000;
    player->GetAbsVelocity() = player->GetVelocity();

    const auto old_invalidation = enable_bone_cache_invalidation;

    csgo->EnableBones = player->GetClientSideAnims() = true;
    resolver->Do(player, record);
    player->UpdateClientSideAnimation();
    csgo->EnableBones = player->GetClientSideAnims() = false;

    player->InvalidatePhysicsRecursive(ANGLES_CHANGED);
    player->InvalidatePhysicsRecursive(ANIMATION_CHANGED);
    player->InvalidatePhysicsRecursive(SEQUENCE_CHANGED);

    enable_bone_cache_invalidation = old_invalidation;

    interfaces.global_vars->curtime = backup_curtime;
    interfaces.global_vars->frametime = backup_frametime;

    player->GetFlagsPtr() = old_flags;
}