#pragma once

#include "profiler.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
//-----------------------------------------Size Limits
    const float Minimum_Actor_Scale = 0.04f;
//-----------------------------------------Default Trigger Threshold Values
    const float Action_Sandwich = 6.0f; // used for sandwich only
	const float Action_Crush = 10.0f;
	const float Action_Booty = 2.0f; // for butt and cleavage crush
	const float Action_Vore = 8.0f;
	const float Action_Grab = 8.0f;
	const float Action_Hug = 0.92f; // for hug grab/drop threshold
    const float Action_FingerGrind = 6.0f;

//-----------------------------------------Default Damage Values

    const float Damage_Grab_Attack = 1.6f;
    ////////////////Defaults

    const float Damage_Default_Underfoot = 0.004f; // when we just stand still

    const float Damage_Walk_Defaut = 8.0f; // when we walk around normally
    const float Damage_Jump_Default = 10.0f; // when we jump land

	const float Damage_Stomp = 10.0f;
	const float Damage_Stomp_Strong = 20.0f;

    /////////////////Foot Grind

    const float Damage_Foot_Grind_Impact = 6.8f;
    const float Damage_Foot_Grind_Rotate = 1.4f;
    const float Damage_Foot_Grind_DOT = 0.048f;

    ////////////////Trample

	const float Damage_Trample = 4.0f;
	const float Damage_Trample_Repeat = 5.0f;
	const float Damage_Trample_Finisher = 22.0f;

    ////////////////Butt Crush

	const float Damage_ButtCrush_ButtImpact = 32.0f;
    const float Damage_ButtCrush_HandImpact = 6.0f;

    const float Damage_ButtCrush_FootImpact = 6.0f;

    ////////////////Thigh Sandwich
    const float Damage_ThighSandwich_Impact = 0.5f;
    const float Damage_ThighSandwich_DOT = 0.004f;

    ////////////////Thigh Crush
    const float Damage_ThighCrush_Stand_Up = 8.0f;
    const float Damage_ThighCrush_Butt_DOT = 0.003f;
    const float Damage_ThighCrush_Legs_Idle = 0.0012f;
    const float Damage_ThighCrush_CrossLegs_Out = 3.4f; 
    const float Damage_ThighCrush_CrossLegs_In = 4.6f; 
    const float Damage_ThighCrush_CrossLegs_FeetImpact = 2.8f;

    ////////////////breast

    const float Damage_BreastCrush_Body = 26.0f; // for body impact
	const float Damage_BreastCrush_Impact = 32.0f; // when doing impact
    const float Damage_BreastCrush_BodyDOT = 0.004f; // damage under body
    const float Damage_BreastCrush_BreastDOT = 0.010f; // damage under breasts

    ////////////////Knee

	const float Damage_KneeCrush = 32.0f;

    ////////////////kick

	const float Damage_Kick = 5.0f;
	const float Damage_Kick_Strong = 12.0f;

    ////////////////crawl

    const float Damage_Crawl_Idle = 0.04f;

    const float Damage_Crawl_KneeImpact_Drop = 14.0f;
    const float Damage_Crawl_HandImpact_Drop = 12.0f;

    const float Damage_Crawl_KneeImpact = 8.0f;
    const float Damage_Crawl_HandImpact = 6.0f;

	const float Damage_Crawl_HandSwipe = 5.0f;
	const float Damage_Crawl_HandSwipe_Strong = 10.0f;

    const float Damage_Crawl_HandSlam = 10.0f;
    const float Damage_Crawl_HandSlam_Strong = 18.0f;

    const float Damage_Crawl_Vore_Butt_Impact = 32.0f;

    ////////////////sneaking

	const float Damage_Sneak_HandSwipe = 4.5f;
	const float Damage_Sneak_HandSwipe_Strong = 10.0f;

    const float Damage_Sneak_HandSlam = 4.0f;
    const float Damage_Sneak_HandSlam_Strong = 20.0f;
    const float Damage_Sneak_HandSlam_Strong_Secondary = 2.6f;

    const float Damage_Sneak_FingerGrind_DOT = 0.0032f;
    const float Damage_Sneak_FingerGrind_Impact = 3.0f;
    const float Damage_Sneak_FingerGrind_Finisher = 6.0f;

    ////////////////Throw

    const float Damage_Throw_Collision = 10.0f; // unused for now, buggy

    

//-----------------------------------------Default effect radius variables

    const float Radius_Default_Idle = 6.2f;

    const float Radius_Walk_Default = 6.6f;
    const float Radius_Jump_Default = 18.0f; 

    const float Radius_Stomp = 6.6f;
    const float Radius_Stomp_Strong = 7.0f;

    /////////Foot Grind
    const float Radius_Foot_Grind_Impact = 7.2f;
    const float Radius_Foot_Grind_DOT = 9.0f;

    /////////Foot Trample
    const float Radius_Trample = 6.6f;
    const float Radius_Trample_Repeat = 6.8f;
    const float Radius_Trample_Finisher = 7.6f;

    /////////Butt Crush

    const float Radius_ButtCrush_Impact = 20.0f;
    const float Radius_ButtCrush_HandImpact = 8.0f;
    const float Radius_ButtCrush_FootImpact = 7.2f;

    /////////Thigh Crush
    const float Radius_ThighCrush_Butt_DOT = 11.0f;
    const float Radius_ThighCrush_Idle = 7.2f;
    
    const float Radius_ThighCrush_Spread_In = 9.0f;
    const float Radius_ThighCrush_Spread_Out = 8.5f;

    const float Radius_ThighCrush_ButtImpact = 16.0f;
    const float Radius_ThighCrush_Stand_Up = 6.2f;
    
    ////////Breast Crush

    const float Radius_BreastCrush_BodyImpact = 16.0f;
    const float Radius_BreastCrush_BreastImpact = 16.0f;
    const float Radius_BreastCrush_BodyDOT = 14.0f; 
    const float Radius_BreastCrush_BreastDOT = 14.0f; 

    ///////Proning

    const float Radius_Proning_BodyDOT = 10.0f;

    ////////Crawling
    const float Radius_Crawl_HandSwipe = 20.0f;
    const float Radius_Crawl_KneeImpact = 14.0f;
    const float Radius_Crawl_HandImpact = 12.0f;

    const float Radius_Crawl_KneeImpact_Fall = 18.0f;
    const float Radius_Crawl_HandImpact_Fall = 14.0f;

    const float Radius_Crawl_Slam = 10.0f;
    const float Radius_Crawl_Slam_Strong = 10.0f;

    const float Radius_Crawl_KneeIdle = 7.4f;
    const float Radius_Crawl_HandIdle = 7.2f;

    const float Radius_Crawl_Vore_ButtImpact = 20.0f;

    ///////Sneaking
    const float Radius_Sneak_HandSwipe = 18.0f;
    const float Radius_Sneak_KneeCrush = 16.0f;
    const float Radius_Sneak_HandSlam = 10.0f;
    const float Radius_Sneak_HandSlam_Strong = 10.0f;
    const float Radius_Sneak_HandSlam_Strong_Recover = 8.0f;

    const float Radius_Sneak_FingerGrind_DOT = 4.2f;
    const float Radius_Sneak_FingerGrind_Impact = 4.6f;
    const float Radius_Sneak_FingerGrind_Finisher = 5.0f;

    ///////Kicks
    const float Radius_Kick = 20.0f;
    /////////////////////////////////////////////////////

}