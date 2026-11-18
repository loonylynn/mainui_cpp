/*
 * Project-specific options for the Xash3D / Half-Life custom build.
 * Keep these controls in mainui rather than requiring console commands.
 */

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "CheckBox.h"
#include "SpinControl.h"
#include "StringArrayModel.h"

#define ART_BANNER "gfx/shell/head_vidoptions"

static const char *s_MSAAModeNames[] =
{
	"Disabled",
	"2x MSAA",
	"4x MSAA",
	"8x MSAA",
	"16x MSAA"
};

static const int s_MSAASamples[] = { 0, 2, 4, 8, 16 };
static CStringArrayModel s_MSAAModeModel( s_MSAAModeNames, V_ARRAYSIZE( s_MSAAModeNames ) );

class CMenuProjectFeatures : public CMenuFramework
{
public:
	CMenuProjectFeatures() : CMenuFramework( "CMenuProjectFeatures" ), m_bGLActive( false ) { }

private:
	void _Init() override;
	void Reload() override;
	void SaveAndPopMenu() override;
	void GetMSAA();
	void WriteMSAA();
	void GetViewBobbing();
	void WriteViewBobbing();
	void WriteInteractionBrackets();

	bool m_bGLActive;

	CMenuSpinControl msaa;
	CMenuCheckBox viewBobbing;
	CMenuCheckBox interactionBrackets;
	CMenuCheckBox npcRagdolls;
	CMenuCheckBox lowViolence;
};

void CMenuProjectFeatures::GetMSAA()
{
	int samples = (int)EngFuncs::GetCvarFloat( "gl_msaa_samples" );
	if( EngFuncs::GetCvarFloat( "gl_msaa" ) < 0.5f )
		samples = 0;

	int index = 0;
	for( int i = 0; i < V_ARRAYSIZE( s_MSAASamples ); i++ )
	{
		if( samples == s_MSAASamples[i] )
		{
			index = i;
			break;
		}
	}

	msaa.SetCurrentValue( (float)index );
}

void CMenuProjectFeatures::WriteMSAA()
{
	int index = bound( 0, (int)msaa.GetCurrentValue(), V_ARRAYSIZE( s_MSAASamples ) - 1 );
	int samples = s_MSAASamples[index];

	// Keep the window framebuffer and renderer off-screen MSAA switches in sync.
	EngFuncs::CvarSetValue( "gl_msaa", samples > 0 ? 1.0f : 0.0f );
	EngFuncs::CvarSetValue( "gl_msaa_samples", (float)samples );
}

void CMenuProjectFeatures::GetViewBobbing()
{
	// Treat the camera bob and both viewmodel motion components as one option.
	// If any component is enabled in an existing config, display the option as on.
	const bool enabled =
		EngFuncs::GetCvarFloat( "cl_viewbob" ) > 0.0f ||
		EngFuncs::GetCvarFloat( "cl_hl2_viewmodel_bob" ) > 0.0f ||
		EngFuncs::GetCvarFloat( "cl_hl2_viewmodel_lag" ) > 0.0f;

	viewBobbing.SetCvarValue( enabled ? 1.0f : 0.0f );
}

void CMenuProjectFeatures::WriteViewBobbing()
{
	// Keep the original implementation values fixed; the menu exposes only
	// a single on/off choice, not individual tuning controls.
	const float value = viewBobbing.CvarValue() > 0.5f ? 1.0f : 0.0f;

	EngFuncs::CvarSetValue( "cl_viewbob", value );
	EngFuncs::CvarSetValue( "cl_hl2_viewmodel_bob", value );
	EngFuncs::CvarSetValue( "cl_hl2_viewmodel_lag", value );
}

void CMenuProjectFeatures::WriteInteractionBrackets()
{
	// Preserve the established 0.5 scale; the option is only enabled/disabled.
	const float scale = interactionBrackets.CvarValue() > 0.5f ? 0.5f : 0.0f;
	EngFuncs::CvarSetValue( "sv_useable_bracket_scale", scale );
}

void CMenuProjectFeatures::SaveAndPopMenu()
{
	if( m_bGLActive )
		msaa.WriteCvar();

	viewBobbing.WriteCvar();
	interactionBrackets.WriteCvar();
	npcRagdolls.WriteCvar();
	lowViolence.WriteCvar();

	CMenuFramework::SaveAndPopMenu();
}

void CMenuProjectFeatures::_Init()
{
	banner.SetPicture( ART_BANNER );

	msaa.szName = L( "Anti-aliasing" );
	msaa.szStatusText = L( "Applying changes recreates the video context" );
	msaa.Setup( &s_MSAAModeModel );
	msaa.SetRect( 360, 250, 300, 32 );
	msaa.SetCharSize( QM_SMALLFONT );
	msaa.onCvarGet = VoidCb( &CMenuProjectFeatures::GetMSAA );
	msaa.onCvarWrite = VoidCb( &CMenuProjectFeatures::WriteMSAA );

	viewBobbing.szName = L( "View bobbing" );
	viewBobbing.szStatusText = L( "Enable or disable first-person camera and weapon movement" );
	viewBobbing.SetCoord( 360, 320 );
	viewBobbing.onCvarGet = VoidCb( &CMenuProjectFeatures::GetViewBobbing );
	viewBobbing.onCvarWrite = VoidCb( &CMenuProjectFeatures::WriteViewBobbing );

	interactionBrackets.szName = L( "PS2 interaction brackets" );
	interactionBrackets.szStatusText = L( "Enable or disable interaction brackets" );
	interactionBrackets.SetCoord( 360, 390 );
	interactionBrackets.onCvarWrite = VoidCb( &CMenuProjectFeatures::WriteInteractionBrackets );

	npcRagdolls.szName = L( "NPC articulated ragdolls" );
	npcRagdolls.szStatusText = L( "Use the custom Jolt-backed NPC ragdoll system" );
	npcRagdolls.SetCoord( 360, 460 );

	lowViolence.szName = L( "Low-violence ragdoll fading" );
	lowViolence.szStatusText = L( "Fade retired NPC ragdolls instead of leaving them visible" );
	lowViolence.SetCoord( 360, 530 );

	AddItem( banner );
	AddButton( L( "Apply" ), nullptr, PC_OK, VoidCb( &CMenuProjectFeatures::SaveAndPopMenu ) );
	AddButton( L( "Cancel" ), nullptr, PC_CANCEL, VoidCb( &CMenuFramework::Hide ) );

	// These items must belong to this window before LinkCvar() can invoke
	// parent-bound callbacks via UpdateCvar().
	AddItem( msaa );
	AddItem( viewBobbing );
	AddItem( interactionBrackets );
	AddItem( npcRagdolls );
	AddItem( lowViolence );

	msaa.LinkCvar( "gl_msaa_samples", CMenuEditable::CVAR_VALUE );
	viewBobbing.LinkCvar( "cl_viewbob" );
	interactionBrackets.LinkCvar( "sv_useable_bracket_scale" );
	npcRagdolls.LinkCvar( "sv_npc_vphysics" );
	lowViolence.LinkCvar( "sv_npc_vphysics_low_violence" );
}

void CMenuProjectFeatures::Reload()
{
	m_bGLActive = !strnicmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "gl", 2 );
	msaa.SetGrayed( !m_bGLActive );
	msaa.SetInactive( !m_bGLActive );

	CMenuFramework::Reload();
}

ADD_MENU3( menu_projectfeatures, CMenuProjectFeatures, UI_ProjectFeatures_Menu );

void UI_ProjectFeatures_Menu( void )
{
	menu_projectfeatures->banner.SetPicture( ART_BANNER );
	menu_projectfeatures->szName = L( "Advanced" );
	menu_projectfeatures->Show();
}
