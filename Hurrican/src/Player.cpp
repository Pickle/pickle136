// Datei : Player.cpp

// --------------------------------------------------------------------------------------
//
// Funktionen f�r den Spieler
// Tasteneingaben bearbeiten und Spieler entsprechend animieren
// usw
//
// (c) 2002 J�rg M. Winterstein
//
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------------------

#include <stdio.h>
#include <string>
#include "Console.h"
#include "GUISystem.h"
#include "Player.h"
#include "DX8Graphics.h"
#include "DX8Input.h"
#include "DX8Sound.h"
#include "DX8Font.h"
#include "Gegner_Helper.h"
#include "Globals.h"
#include "Gameplay.h"
#include "Logdatei.h"
#include "Menu.h"
#include "Main.h"
#include "Outtro.h"
#include "Partikelsystem.h"
#include "Projectiles.h"
#include "Tileengine.h"
#include "Timer.h"

extern long DEMOPress;
extern bool	DEMORecording;
extern bool	DEMOPlaying;

extern bool	JoystickFound;

int Stage;				// Aktuelles Level
int NewStage;			// Neues Level
int	Skill;				// 0 = easy, 1 = medium, 2 = hard, 3 = Hurrican

// --------------------------------------------------------------------------------------
// Player-Klasse
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Konstruktor : Spieler initialisieren und Grafiken laden
// --------------------------------------------------------------------------------------

PlayerClass::PlayerClass()
{
    //DKS - New variable PlayerNumber represents which player a class instance
    //      represents. PlayerNumber will be set to 1 if InitPlayer(1) is called,
    //      or will remain 0 as it is here when InitPlayer(0) is called.
    PlayerNumber = 0;       // This will be set to 1 if InitPlayer(1) is called on a class instance.
    SoundOff = 0;           // This will be set to 1 if InitPlayer(1) is called on a class instance.

    //DKS - Added initialization of member vars here, since Main.cpp was just doing a memset to
    //      fill the entire class with 0's and that won't do now that we have the sprites as
    //      member vars.
    AnimCount = 0;
    BlitzStart = 0;
    FlameTime = 0;
    AustrittX = AustrittY = 0;
    AustrittAnim = 0;
    FlameAnim = 0;
    randomwert = 0;
    changecount = 0;
    weaponswitchlock = false;
    GegnerDran = false;
    AlreadyDrawn = false;
    memset(Aktion, 0, sizeof(Aktion));
    memset(AktionKeyboard, 0, sizeof(AktionKeyboard));
    memset(AktionJoystick, 0, sizeof(AktionJoystick));
    Walk_UseAxxis = false;
    Look_UseAxxis = false;
    CanBeDamaged = false;
    GodMode = false;
    WheelMode = false;
    WasDamaged = false;
    memset(&CollideRect, 0, sizeof(CollideRect));
    AufPlattform = NULL;
    InLiquid = false;
    JumpPossible = false;
    JumpedOnSurfboard = false;
    PowerLinePossible = false;
    PunisherActive = false;
    WalkLock = false;
    AnimPhase = 0;
    AnimEnde = 0;
    GameOverTimer = 0;
    ShotDelay = 0;
    AutoFireCount = 0;
    AutoFireExtra = 0;
    RiesenShotExtra = 0;
    xposold = yposold = 0;
    xpos = ypos = 0;
    xspeed = yspeed = 0;
    xadd = yadd = 0;
    BeamX = BeamY = 0;
    BeamCount = 0;
    AutoScrollspeed = 0;
    JumpySave = 0;
    JumpxSave = 0;
    JumpStart = 0;
    JumpAdd = 0;
    SmokeCount = 0;
    BlitzWinkel = 0;
    BlitzCount = 0;
    BlitzAnim = 0;
    Handlung = 0;
    Blickrichtung = 0;
    Energy = 0;
    Armour = 0;
    Shield = 0;
    DamageCounter = 0;
    BlinkCounter = 0;
    BlinkColor = 0;
    CurrentColor = 0;
    Score = 0;
    CollectedDiamonds = 0;
    DiamondsThisLevel = 0;
    DiamondsFullGame = 0;
    LivesThisLevel = 0;
    LivesFullGame = 0;
    BlocksThisLevel = 0;
    BlocksFullGame = 0;
    Lives = 0;
    SelectedWeapon = 0;
    memset(CurrentWeaponLevel, 0, sizeof(CurrentWeaponLevel));
    BlitzLength = 0;
    PowerLines = 0;
    Grenades = 0;
    SmartBombs = 0;
    SecretThisLevel = 0;
    SecretFullGame = 0;
    BronsonCounter = 0;
    FesteAktion = 0;
    DoFesteAktion = false;
    StageClearRunning = false;
    JoystickIndex = 0;
    JoystickSchwelle = 0;
    ControlType = 0;
    JoystickMode = 0;
    ExplodingTimer = 0;

    SpritesLoaded = false;      //DKS - Added this bool so sprites can be loaded on-demand

    //DKS - All the rest of the following were already present in the constructor, but
    //      likely having no effect at all because the game was memsetting each player
    //      class to 0 after allocating each player with new(). I hope there are no
    //      unintended consequences of leaving these at the bottom of the constructor,
    //      where they will now have an effect.
    Stage    = -1;				// Es l�uft noch kein Spiel
    NewStage = -1;
    StageClearRunning = false;
    JoystickSchwelle = 500.0f;
    ControlType = CONTROLTYPE_KEYBOARD;
    JoystickMode = JOYMODE_JOYPAD;
    JoystickIndex = 0;
    GodMode = false;
    WheelMode = false;
    ExplodingTimer = 0.0f;
    BeamCount = 0.0f;
    changecount = 0.0f;
    memset( &strahlen, 0, sizeof(strahlen));
}

// --------------------------------------------------------------------------------------
// Desktruktor
// --------------------------------------------------------------------------------------

PlayerClass::~PlayerClass(void)
{
}

// --------------------------------------------------------------------------------------
// Spieler auf neues Spiel vorbereiten
// --------------------------------------------------------------------------------------

//DKS - InitPlayer now takes an argument the class uses to set which player number
//      a specific class instance is. 0 for Player1, 1 for Player2.
void PlayerClass::InitPlayer(int player_num)
{
    if (player_num == 1) {
        PlayerNumber = 1;
        SoundOff = 1;
    } else {
        PlayerNumber = 0;
        SoundOff = 0;
    }

    //DKS - Load each player's individual set of sprites (Player 1's has red highlights,
    //      Player 2 has blue highlights).
    LoadSprites();

    BronsonCounter			= 0.0f;
    Score					= 0;
    Stage					= 1;
    NewStage				= 1;
    Lives					= 3;
    Armour					= MAX_ARMOUR;
    Energy					= MAX_ENERGY;
    yspeed					= 0.0f;
    yadd					= 0.0f;

    // Waffen initialisieren
    PowerLines				= 3;
    Grenades				= 3;
    SmartBombs				= 1;
    SelectedWeapon			= 0;

    CollectedDiamonds		= 0;

    int i = 0;
    /*
    	for(i=0; i<4; i++)
    		CollectedPowerUps[i] = 0;
    */

    for(i=0; i<4; i++)
    {
        CurrentWeaponLevel[i] = 0;
//		NextWeaponLevel	  [i] = 3;
    }

    CurrentWeaponLevel[0] = 1;

    // Blitz
    BlitzLength			  = 3;			// Aktuelle
    CurrentWeaponLevel[3] = 2;			// L�nge
    //NextWeaponLevel	  [3] = 3;			// Wieviel PowerUps zum n�chsten Level ?

    AutoFireCount   = 0.0f;
    AutoFireExtra   = 0.0f;
    RiesenShotExtra = 0.0f;

    // Spielt der Spieler das TutorialLevel? Dann alle Extra-Waffen auf Null setzen
    //
    if (RunningTutorial == true)
    {
        Skill = 0;
        PowerLines = 0;
        Grenades   = 0;
        SmartBombs = 0;
    }

    GodMode = false;
    WheelMode = false;
    InitNewLevel();
    AutoScrollspeed = 0.0f;
}

// --------------------------------------------------------------------------------------
// Spieler auf neues Level vorbereiten
// --------------------------------------------------------------------------------------

void PlayerClass::InitNewLevel()
{
    if (Handlung == TOT)
        return;

    BronsonCounter			= 0.0f;
    Handlung				= STEHEN;
    JumpAdd					= 0.0f;
    AnimPhase				= 0;
    AnimCount				= 0.0f;
    xspeed				    = 0.0f;
    yspeed					= 0.0f;
    xadd					= 0.0f;
    yadd					= 0.0f;
    Blickrichtung			= RECHTS;
    JumpPossible			= true;
    CollideRect.left		= 20;
    CollideRect.top			= 12;
    CollideRect.right		= 50;
    CollideRect.bottom		= 80;
    Energy					= MAX_ENERGY;
    Armour					= MAX_ARMOUR;

    // Waffen initialisieren
    ShotDelay				= 0.0f;
    BlitzWinkel				= 0.0f;
    BlitzCount				= 0.0f;
    BlitzAnim				= 0;
    PowerLinePossible		= true;

    AufPlattform			= NULL;
    InLiquid				= false;
    JumpedOnSurfboard		= false;

    DamageCounter			= 0.0f;
    Shield					= 0.0f;

    GameOverTimer			= 0.0f;

    FlameTime				= 0.0f;

    SecretThisLevel			= 0;
    DiamondsThisLevel		= 0;
    LivesThisLevel			= 0;
    BlocksThisLevel			= 0;

    CanBeDamaged			= true;
    WalkLock				= true;

    FesteAktion = -1;
    DoFesteAktion = false;
    StageClearRunning = false;


    // Zu Beginn des Levels werden alle Listen mit Gegner, Sch�ssen und Partikeln gel�scht
    PartikelSystem.ClearAll();
    Gegner.ClearAll();
    Projectiles.ClearAll();
}

// --------------------------------------------------------------------------------------
// Spieler explodiert gerade
// --------------------------------------------------------------------------------------

void PlayerClass::runExplode(void)
{
    ExplodingTimer -= 1.0f SYNC;

    // Zuende explodiert?
    //
    if (ExplodingTimer <= 0.0f)
    {
        // Rechteck wieder herstellen
        //
        CollideRect.left		= 20;
        CollideRect.top			= 12;
        CollideRect.right		= 50;
        CollideRect.bottom		= 79;

        // Dann ein Leben abziehen
        Lives--;
        if (Lives < 0)
            Handlung = TOT;

        // Aktuell eingestellte Waffe um eine Stufe verringern
        //
        if (CurrentWeaponLevel[SelectedWeapon] > 1)
            CurrentWeaponLevel[SelectedWeapon]--;

        // Blitz um eine Stufe verk�rzen
        //
        if (CurrentWeaponLevel[3] > 2)
            CurrentWeaponLevel[3]--;

        // Powerlines wieder minimal auf Spieleranzahl setzen
        if (PowerLines < NUMPLAYERS)
            PowerLines = NUMPLAYERS;

        // Granaten wieder minimal auf Spieleranzahl setzen
        if (Grenades < NUMPLAYERS)
            Grenades = NUMPLAYERS;

        //DKS - This was commented out in original source
        // SmartBombs wieder minimal auf 1 setzen
        //if (SmartBombs < 1)
        //	SmartBombs = 1;

        if (TileEngine.Zustand == ZUSTAND_SCROLLBAR &&
                Handlung != SACKREITEN &&
                Handlung != DREHEN)
        {
            xpos    = JumpxSave;	// Alte Position wieder herstellen, wenn der
            ypos    = JumpySave;	// der Spieler zB einen Abgrund runterfiel
            xposold = JumpxSave;
            yposold = JumpySave;
        }

        // Spieler hat Game Over ?
        if (Player[0].Lives < 0 &&
                (NUMPLAYERS == 1 ||
                 Player[1].Lives < 0))
        {
            Player[0].Lives = -1;
            Player[1].Lives = -1;
            Player[0].GameOverTimer = 50.0f;

            //DKS - We should really just stop all songs (it was missing MUSIC_PUNISHER from this list anyway)
            //SoundManager.StopSong(MUSIC_STAGEMUSIC, false);
            //SoundManager.StopSong(MUSIC_FLUGSACK, false);
            //SoundManager.StopSong(MUSIC_BOSS, false);
            SoundManager.StopSongs(); //DKS - Added this to replace above 3 lines

            SoundManager.PlaySong(MUSIC_GAMEOVER, false);
            pMenu->AktuellerZustand = 0;
        }

        // oder wird wieder gespanwed
        else if (Lives >= 0)
        {
            // Spieler hat kurze Zeit lang Schild
            //if (Shield <= 0.0f)
            Projectiles.PushProjectile (xpos, ypos, SHIELDSPAWNER, this);
            Projectiles.PushProjectile (xpos, ypos, SHIELDSPAWNER2, this);

            Shield += 20.0f;

            // Energie und Rad-Energie wieder auf voll setzen
            Energy = MAX_ENERGY;
            Armour = MAX_ARMOUR;

            // Im Fahrstuhllevel? Dann auf dem Fahrstuhl wieder anfangen
            //
            if (g_Fahrstuhl_yPos > -1.0f)
            {
                xpos = (float)(TileEngine.XOffset) + 320.0f - 35;
                ypos = (float)(g_Fahrstuhl_yPos) - CollideRect.bottom - 1;
                xposold = xpos;	// Alte Position wieder herstellen, wenn der
                yposold = ypos;	// der Spieler zB einen Abgrund runterfiel
                JumpxSave = xpos;
                JumpySave = ypos;
            }
        }

        if (Lives < -1)
            Lives = -1;

        if (Handlung != SACKREITEN &&
                Handlung != DREHEN &&
                Handlung != TOT)
            Handlung = SPRINGEN;

        yspeed   = 0.0f;
        JumpAdd  = 0.0f;
    }
}

// --------------------------------------------------------------------------------------
// will der Spieler schiessen?
// --------------------------------------------------------------------------------------

void PlayerClass::checkShoot(void)
{
    //DKS - No need to check if sound is playing, just ask to stop it:
    //if (!Aktion[AKTION_SHOOT] &&
    //        FlameThrower == true &&
    //        SoundManager.its_Sounds[SOUND_FLAMETHROWER + SoundOff]->isPlaying)
    //    SoundManager.StopWave(SOUND_FLAMETHROWER + SoundOff);
    if (!Aktion[AKTION_SHOOT] && FlameThrower == true)
        SoundManager.StopWave(SOUND_FLAMETHROWER + SoundOff);

    // Normaler Schuss mit Prim�rwaffe?
    //
    if(Aktion[AKTION_SHOOT]  &&
            Handlung != BLITZEN   &&
            Handlung != BEAMLADEN &&
            Handlung != DREHEN    &&
            AutoFireCount > 0.0f  &&
            ShotDelay <= 0.0f)
    {
        ShotDelay = PLAYER_SHOTDELAY;
        PlayerShoot();

        if (Handlung != LAUFEN	    && Handlung != SPRINGEN &&
                Handlung != SACKREITEN  && Handlung != RADELN   &&
                Handlung != RADELN_FALL &&
                WalkLock == true)
            AnimPhase = 1;

        // TODO immer Dauerfeuer?
        /*// Nur bei "Easy" Dauerfeuer
        // ansonsten Autofire Count verringern
        //
        if (Skill > 0)
        	AutoFireCount -= 1.0f;*/

        if (AutoFireExtra > 0.0f)
            ShotDelay /= 2.25f;

        // Beim Sackreiten den Spieler durch den R�ckschlag noch bewegen
        //
        if (Handlung == SACKREITEN)
        {
            if (Blickrichtung == LINKS)
                xadd += 2.0f;
            else
                xadd -= 2.0f;
        }

        //DKS - This appears never to have been implemented (no image for it) so disabled it:
#if 0
        // Beim Surfen genauso ;)
        //
        if (Handlung == SURFEN     ||
                Handlung == SURFENJUMP ||
                Handlung == SURFENCROUCH)
            xadd -= 2.0f;
#endif //0
    }

    // Granate abfeuern ?
    //
    if (Aktion[AKTION_GRANATE] == true  &&
            PowerLinePossible == true       &&
            Grenades > 0		&&
            Handlung != RADELN				&&
            Handlung != RADELN_FALL			&&
            Handlung != BLITZEN				&&
            Handlung != DREHEN				&&
            Handlung != BEAMLADEN)
    {
        PlayerGrenadeShoot();
    }

    // Kann man wieder schiessen ?
    //
    if (Aktion[AKTION_SHOOT] == false)
    {
        ShotDelay     = 0.0f;

        if (AutoFireCount < 3.0f)
            AutoFireCount = 3.0f;
    }

    if (ShotDelay > 0.0f)
        ShotDelay -= float(1.0 SYNC);
    else
        ShotDelay = 0.0f;
}

// --------------------------------------------------------------------------------------
// Autofire abhandeln
// --------------------------------------------------------------------------------------

void PlayerClass::handleAutoFire (void)
{
    // Eingesammeltes Autofire runterz�hlen
    //
    if (AutoFireExtra > 0.0f)
    {
        AutoFireExtra -= 0.5f SYNC;
        AutoFireCount  = 3;
    }
    else
        AutoFireExtra = 0.0f;

    // Eingesammeltes RiesenShotExtra runterz�hlen
    //
    if (RiesenShotExtra > 0.0f)
        RiesenShotExtra -= 0.5f SYNC;
    else
        RiesenShotExtra = 0.0f;
}

// --------------------------------------------------------------------------------------
// Eingabeger�te abfragen und Spieler entsprechend reagieren lassen
// --------------------------------------------------------------------------------------

bool PlayerClass::GetPlayerInput(void)
{
    // tot?
    if (Handlung == TOT)
        return false;

    // explodiert der Spieler noch?
    //
    if (Handlung == EXPLODIEREN)
    {
        runExplode();
        return false;
    }

    // Zuerst alle Aktionen auf false setzen
    //
    for (int i=0; i<MAX_AKTIONEN; i++)
        Aktion[i] = false;

    // und Bewegungsgeschwindigkeit f�r den n�chsten Frame auf 0 setzen,
    // es sei denn, man l�uft auf Eis
    if (TileEngine.BlockUntenNormal	  (xpos, ypos, xposold, yposold, CollideRect) & BLOCKWERT_EIS)
    {
        xspeed *= PLAYER_ICESSLOWDOWN;
    }
    else
        xspeed = 0.0f;

    // Aktionen nach Keyboard-Input abfragen falls der
    // Spieler sich gerade bewegen darf =)
    //
//	if (TileEngine.Zustand != ZUSTAND_SCROLLTOPLAYER)
    {
        // Demo l�uft ?
        if (DEMOPlaying == true)
            PlayDemo();

        // oder feste Aktion?
        else if (DoFesteAktion == true)
        {
            if (FesteAktion >= 0)
                Aktion[FesteAktion] = true;
        }
        // oder Spieler spielt ?
        else
        {
            //DKS - Cleaned a lot of this up and made the joystick support more complete,
            //      especially in regards to Walk_UseAxxis and Look_UseAxxis,
            //      along with safeguarding against checking button inputs on actions that
            //      are undefined, and not checking the keyboard for actions unless the
            //      player is set to use it as the control method.

            // Keyboard
            if (ControlType == CONTROLTYPE_KEYBOARD)
            {
                for (int i=0; i<MAX_AKTIONEN; i++)
                    if (KeyDown(AktionKeyboard[i]))
                    {
                        Aktion[i] = true;
                        BronsonCounter = 0.0f;
                    }
            } else if (JoystickIndex >= 0 && JoystickIndex < DirectInput.JoysticksFound &&
                        DirectInput.Joysticks[JoystickIndex].Active)
            {
                bool stick_right = DirectInput.Joysticks[JoystickIndex].JoystickX >  JoystickSchwelle;
                bool stick_left  = DirectInput.Joysticks[JoystickIndex].JoystickX < -JoystickSchwelle;
                bool stick_up    = DirectInput.Joysticks[JoystickIndex].JoystickY < -JoystickSchwelle;
                bool stick_down  = DirectInput.Joysticks[JoystickIndex].JoystickY >  JoystickSchwelle;
                bool hat_right = false;
                bool hat_left  = false;
                bool hat_up    = false;
                bool hat_down  = false;

                if (DirectInput.Joysticks[JoystickIndex].JoystickPOV != -1) {
                    // HAT switch is pressed
                    if     (DirectInput.Joysticks[JoystickIndex].JoystickPOV     >= 4500 * 1 &&
                            DirectInput.Joysticks[JoystickIndex].JoystickPOV     <= 4500 * 3) {
                        hat_right = true;
                    } else if (DirectInput.Joysticks[JoystickIndex].JoystickPOV  >= 4500 * 5 &&
                            DirectInput.Joysticks[JoystickIndex].JoystickPOV     <= 4500 * 7) {
                        hat_left = true;
                    }

                    if (DirectInput.Joysticks[JoystickIndex].JoystickPOV         >= 4500 * 3 &&
                            DirectInput.Joysticks[JoystickIndex].JoystickPOV     <= 4500 * 5) {
                        hat_down = true;
                    } else if ((DirectInput.Joysticks[JoystickIndex].JoystickPOV >= 4500 * 7 && 
                                DirectInput.Joysticks[JoystickIndex].JoystickPOV <= 4500 * 8) ||
                            (DirectInput.Joysticks[JoystickIndex].JoystickPOV    >= 0        &&
                             DirectInput.Joysticks[JoystickIndex].JoystickPOV    <= 4500 * 1)) {
                        hat_up = true;
                    }
                }

                // Zum laufen die Achse links/rechts auslesen
                if (Walk_UseAxxis) {
                    // Analog stick input used for walking:
                    if (stick_right) Aktion[AKTION_RECHTS] = true;
                    if (stick_left)  Aktion[AKTION_LINKS]  = true;
                    if (stick_down)  Aktion[AKTION_DUCKEN] = true;

                    if (stick_up) {
                        if (JoystickMode == JOYMODE_JOYSTICK) {
                        // Analog stick up-input used for jumping (when in "joystick" mode):
                        // Spieler kann im Joystick Mode nicht mit dem Button springen
                        //
                            Aktion[AKTION_JUMP] = true;
                        } else {
                        // Analog stick up-input used for looking up (when in "joypad" mode)
                            Aktion[AKTION_OBEN] = true;
                        }
                    }
                } else {
                    // HAT/DPAD used for walking:
                    if (hat_right) Aktion[AKTION_RECHTS] = true;
                    if (hat_left)  Aktion[AKTION_LINKS]  = true;
                    if (hat_down)  Aktion[AKTION_DUCKEN] = true;

                    if (hat_up) {
                        if (JoystickMode == JOYMODE_JOYSTICK)
                        // HAT/DPAD up-input used for jumping (when in "joystick" mode):
                        // Spieler kann im Joystick Mode nicht mit dem Button springen
                        //
                            Aktion[AKTION_JUMP] = true;
                        else
                        // HAT/DPAD up-input used for looking up (when in "joypad" mode)
                            Aktion[AKTION_OBEN] = true;
                    }
                }

                if (Look_UseAxxis) {
                    // Analog stick used for looking:
                    if (stick_down)
                        Aktion[AKTION_UNTEN] = true;
                    else if (stick_up)
                        Aktion[AKTION_OBEN]  = true;
                } else {
                    // HAT/DPAD used for looking:
                    if (hat_down)
                        Aktion[AKTION_UNTEN] = true;
                    else if (hat_up)
                        Aktion[AKTION_OBEN]  = true;
                }

                // Joystick buttons auslesen
                for (int i = AKTION_LINKS; i <= AKTION_WAFFEN_CYCLE; i++) {
                    if (AktionJoystick[i] != -1 &&
                            DirectInput.Joysticks[JoystickIndex].JoystickButtons[AktionJoystick[i]]) {
                        if (!(i == AKTION_JUMP && JoystickMode == JOYMODE_JOYSTICK)) 
                            Aktion[i] = true;
                    }
                }
            }

            // Bewegungen aufnehmen ?
            if (DEMORecording == true)
                RecordDemo();
        }
    }

    // Spieler l�uft ins Exit?

    if (Player[0].StageClearRunning == true)
        RunPlayerExit();

    // Spieler schiesst?
    //
    checkShoot();

    // Autofire und Riesenshot Extra abgandeln
    //
    handleAutoFire();

    // waffe wechseln?
    checkWeaponSwitch();


    for (int i=0; i<MAX_AKTIONEN; i++)
        if (Aktion[i])
        {
            if (Handlung == PISSEN)
            {
                GUI.HideBoxFast();
                BronsonCounter = 0.0f;
                Handlung = STEHEN;
            }
        }

    // Bronson-Counter erh�hen
    if ((Handlung == STEHEN ||
            Handlung == PISSEN) &&
            RunningTutorial == false)
        BronsonCounter += 1.0f SYNC;
    else
    {
        BronsonCounter = 0.0f;

        if (Handlung == PISSEN)
            Handlung = STEHEN;
    }

    // und evtl auch mit Bronson loslegen =)
    //
    if (BronsonCounter > 200.0f)
    {
        if (Handlung != PISSEN)
        {
            AnimCount = 0.0f;
            AnimPhase = 0;
            Handlung = PISSEN;
        }
    }

    return true;
}

// --------------------------------------------------------------------------------------
// Funken und Rauch erzeugen, wenn der Spieler angeschlagen ist
// --------------------------------------------------------------------------------------

void PlayerClass::DoStuffWhenDamaged(void)
{
    if (Console.Showing == true ||
            Handlung == EXPLODIEREN)
        return;

    static float sparkcount = 0.0f;
    static float smokecount = 0.0f;

    // Noch viel Energie? Dann gleich wieder raus
    //
    if (Energy > MAX_ENERGY / 2)
        return;

    // Funkenz�hler runterz�hlen
    //
    if (sparkcount > 0.0f)
        sparkcount -= 1.0f SYNC;
    else
    {
        sparkcount = (float)(rand()%(int)(Energy / 2 + 2)) + 5;

        // ein Funken "Schadenseffekt" per Zufall einbauen
        //
        int effect = rand()%3;

        switch (effect)
        {
        // Normale Funken
        //
        case 0:
        {
            float x = xpos + (float)(20 + rand()%40);
            float y = ypos + (float)(20 + rand()%40);

            for (int i = 0; i < 5; i++)
                PartikelSystem.PushPartikel(x + rand()%4, y + rand()%4, FUNKE);

            PartikelSystem.PushPartikel(x - 20, y - 20, LASERFLAME);
            SoundManager.PlayWave(100, 128, 8000 + rand()%4000, SOUND_FUNKE);
        }
        break;

        // Lange Funken
        //
        case 1:
        {
            float x = xpos + (float)(20 + rand()%40);
            float y = ypos + (float)(20 + rand()%40);

            for (int i = 0; i < 5; i++)
                PartikelSystem.PushPartikel(x + rand()%4, y + rand()%4, LONGFUNKE);

            PartikelSystem.PushPartikel(x - 20, y - 20, LASERFLAME);
            SoundManager.PlayWave(100, 128, 8000 + rand()%4000, SOUND_FUNKE);
        }
        break;

        }
    }

    // Rauchz�hler runterz�hlen
    //
    if (smokecount > 0.0f)
        smokecount -= 1.0f SYNC;
    else
    {
        smokecount = 0.8f;

        // Rauch per Zufall
        //
        if (Energy < MAX_ENERGY / 3 + 5.0f)
            if (rand()%2 == 0)
                PartikelSystem.PushPartikel(xpos + 10 + rand()%30, ypos + 20 + rand()%40, SMOKE2);

        // Rauchs�ule
        //
        if (Energy < MAX_ENERGY / 4 + 5.0f)
            PartikelSystem.PushPartikel(xpos + 26 + Blickrichtung * 4 + rand()%4, ypos + 20 + rand()%4, SMOKE3);

    }



}

// --------------------------------------------------------------------------------------
// Energie testen, ob <= 0. Dann explodieren lassen
// --------------------------------------------------------------------------------------

void PlayerClass::CheckForExplode(void)
{
    if (Energy		  <= 0.0f		 &&
            Handlung	  != EXPLODIEREN &&
            GameOverTimer  == 0.0f)
    {
        //DKS - No need to check if the sounds are playing, just request they stop:
        //if (SoundManager.its_Sounds[SOUND_ABZUG + SoundOff]->isPlaying == true)
        //    SoundManager.StopWave(SOUND_ABZUG + SoundOff);

        //if (SoundManager.its_Sounds[SOUND_BLITZ + SoundOff]->isPlaying == true)
        //    SoundManager.StopWave(SOUND_BLITZ + SoundOff);

        //if (SoundManager.its_Sounds[SOUND_BEAMLOAD + SoundOff]->isPlaying == true)
        //    SoundManager.StopWave(SOUND_BEAMLOAD + SoundOff);
        SoundManager.StopWave(SOUND_ABZUG + SoundOff);
        SoundManager.StopWave(SOUND_BLITZ + SoundOff);
        SoundManager.StopWave(SOUND_BEAMLOAD + SoundOff);

        DirectInput.Joysticks[JoystickIndex].StopForceFeedbackEffect(FFE_BLITZ);

        // Screen bei evtl. Ruckeln begradigen
        D3DXMATRIX		matRot;				// Rotationsmatrix
        WackelMaximum = 0.0f;
        WackelValue   = 0.0f;
        D3DXMatrixRotationZ  (&matRot, 0.0f);
#if defined(PLATFORM_DIRECTX)
        lpD3DDevice->SetTransform(D3DTS_WORLD, &matRot);
#elif defined(PLATFORM_SDL)
        g_matModelView = matRot * g_matView;
#if defined(USE_GL1)
        load_matrix( GL_MODELVIEW, g_matModelView.data() );
#endif
#endif

        CollideRect.left   = 0;
        CollideRect.right  = 0;
        CollideRect.top    = 0;
        CollideRect.bottom = 0;

        // Spieler explodieren lassen und Gegnern dabei Schaden zuf�gen
        //
        SoundManager.PlayWave (100, 128, 11025, SOUND_EXPLOSION2);
        Gegner.DamageEnemiesonScreen (xpos + 35, ypos + 40, 400);

        // Piss-Meldung verschwinden lassen
        GUI.HideBoxFast();
        BronsonCounter = 0.0f;

        Shield = 0.0f;

        ShakeScreen (5);

        for (int i = 0; i < 3; i++)
            PartikelSystem.PushPartikel (xpos + 10 + rand () % 50,
                                           ypos + 10 + rand () % 50, SPLITTER);

        for (int i = 0; i < 10; i++)
            PartikelSystem.PushPartikel (xpos + 10 + rand () % 50,
                                           ypos + 10 + rand () % 50, SPIDERSPLITTER);


        PartikelSystem.PushPartikel (xpos + 35 - 90, ypos + 40 - 90, EXPLOSION_GIGA);
        PartikelSystem.PushPartikel (xpos + 35, ypos + 40, SHOCKEXPLOSION);
        PartikelSystem.PushPartikel (xpos - 20, ypos - 20, EXPLOSIONFLARE);

        //DKS - Player 2 sprite is blue, so I added separate particles and particle art for them
        //      that are colored blue, and PlayerClass now tracks which player it is assigned to.
        if (PlayerNumber == 0) {
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_KOPF);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_ARM1);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_ARM2);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_BEIN1);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_BEIN2);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_WAFFE);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_TORSO);
        } else {
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_P2_KOPF);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_P2_ARM1);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_P2_ARM2);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_P2_BEIN1);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_P2_BEIN2);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_P2_WAFFE);
            PartikelSystem.PushPartikel (xpos + 20, ypos + 10, HURRITEILE_P2_TORSO);
        }

        PartikelSystem.PushPartikel (xpos - 88, ypos - 88, GRENADEFLARE);

        ExplodingTimer = 30.0f;

        // Punisher verschwinden lassen
        GegnerClass *pTemp;
        GegnerPunisher *pPunisher;

        pTemp = Gegner.pStart;

        while (pTemp != NULL)
        {
            if (pTemp->GegnerArt == PUNISHER)
            {
                pPunisher = (GegnerPunisher*)pTemp;
                pPunisher->Vanish();
            }

            pTemp = pTemp->pNext;
        }

        if (Handlung == SACKREITEN ||
                Handlung == DREHEN)
        {
            // Von unten wieder hochkommen ?
            if (ypos >= TileEngine.YOffset + 475.0f)
            {
                ypos = float (TileEngine.YOffset + 470.0f);
                yadd   = -25.0f;
            }

            ExplodingTimer = 0.0f;
            SoundManager.PlayWave (100, 128, 11025, SOUND_EXPLOSION2);

            for (int i = 0; i < 15; i++)
                PartikelSystem.PushPartikel(xpos - 10 + rand()%80,
                                              ypos - 10 + rand()%80,
                                              EXPLOSION_MEDIUM2);

            for (int i = 0; i < 40; i++)
                PartikelSystem.PushPartikel(xpos + rand()%90,
                                              ypos + rand()%90,
                                              LONGFUNKE);
            runExplode();
        }
        else if (Handlung != TOT)
            Handlung = EXPLODIEREN;

        DirectInput.Joysticks[JoystickIndex].ForceFeedbackEffect(FFE_BIGRUMBLE);
        InLiquid = false;
    }
}

// --------------------------------------------------------------------------------------
// Items in Reichweite anziehen
// --------------------------------------------------------------------------------------

void PlayerClass::PullItems(void)
{
    GegnerClass* pTemp;

    pTemp = Gegner.pStart;

    while (pTemp != NULL)
    {
        // Item ist ein Extra? Dann ansaugen
        if (pTemp->GegnerArt == EXTRAS &&
                pTemp->Handlung == GEGNER_STEHEN &&
                pTemp->IsOnScreen())
        {
            float absx, absy, speed;				// Variablen f�r die Geschwindigkeits-
            // berechnung
            absx = (pTemp->xPos+10)-(xpos+35);		// Differenz der x
            absy = (pTemp->yPos+10)-(ypos+40);		// und y Strecke

            //DKS - converted to float:
            //speed = (float)(1.0f/sqrt(absx*absx + absy*absy));	// L�nge der Strecke berechnen
            speed = 1.0f/sqrtf(absx*absx + absy*absy);	// L�nge der Strecke berechnen
            speed = speed * 0.1f * BlitzStart;				// Geschwindigkeit ist 4 fach

            pTemp->xSpeed = -speed * 10.0f * absx SYNC;
            pTemp->yPos -= speed * absy SYNC;
        }

        pTemp = pTemp->pNext;
    }
}

// --------------------------------------------------------------------------------------
// Spieler animieren
// --------------------------------------------------------------------------------------

void PlayerClass::AnimatePlayer(void)
{
    // Pause beim Explodieren
    //
    if (Handlung == EXPLODIEREN ||
            Handlung == TOT)
        return;

    static float	look;		// hoch / runter sehen

    //DKS - Added so we can limit bounds of accesses to Tiles[][] array here
    int levelsize_x = TileEngine.LEVELSIZE_X;
    int levelsize_y = TileEngine.LEVELSIZE_Y;

    bu = 0;

    bl = TileEngine.BlockLinks	  (xpos, ypos, xposold, yposold, CollideRect, yspeed >= 0.0f);
    br = TileEngine.BlockRechts	  (xpos, ypos, xposold, yposold, CollideRect, yspeed >= 0.0f);

    // auf schr�gen laufen/gelandet?
    //
    if (yspeed   >= 0.0f)
        //DKS - Rewrote BlockSlopes function to only take the parameters it needs:
        //bu = TileEngine.BlockSlopes     (xpos, ypos, xposold, yposold, CollideRect, yspeed);
        bu = TileEngine.BlockSlopes     (xpos, ypos, CollideRect, yspeed);

    if (Handlung == RADELN)
    {
        if ((Blickrichtung == LINKS   && bu & BLOCKWERT_SCHRAEGE_R) ||
                (Blickrichtung == RECHTS  && bu & BLOCKWERT_SCHRAEGE_L))
        {
            bu = TileEngine.BlockUnten (xpos, ypos, xposold, yposold, CollideRect, yspeed >= 0.0f);
            yspeed = -2.0f;
            ypos -= 1.0f;

        }
    }

    // normaler Boden
    //
    if (!(bu & BLOCKWERT_SCHRAEGE_L) &&
            !(bu & BLOCKWERT_SCHRAEGE_R))
    {
        bu = TileEngine.BlockUnten	  (xpos, ypos, xposold, yposold, CollideRect, yspeed >= 0.0f);
        bu = TileEngine.BlockUntenNormal(xpos, ypos, xposold, yposold, CollideRect);
    }
    else
    {
        if (Handlung == SPRINGEN)
        {
            Handlung = STEHEN;
            yspeed   = 0.0f;
            JumpAdd  = 0.0f;
        }
    }

    bo = TileEngine.BlockOben		  (xpos, ypos, xposold, yposold, CollideRect, true);

    // Spieler unter Wasserfall ? Dann Wassertropfen entstehen lassen
    //DKS - Added bounds check for access to Tiles[][] array, eliminated divisions while I was here:
#if 0
    for (int i = int (xpos + CollideRect.left) + 5; i < int (xpos + CollideRect.right) - 5; i+=2)
    {
        if (TileEngine.TileAt(int (i / 20), int (ypos + CollideRect.top + 10) / 20).Block & BLOCKWERT_WASSERFALL)
        {
            if (rand()%50 == 0)
                PartikelSystem.PushPartikel (float (i) + rand()%4, ypos + CollideRect.top + rand()%4 + 10, WASSERTROPFEN);

            if (rand()%200 == 0)
                PartikelSystem.PushPartikel (float (i) + rand()%4, ypos + CollideRect.top + rand()%4 + 10, WATERFUNKE);

            if (rand()%200 == 0)
                PartikelSystem.PushPartikel (float (i) + rand()%4 - 16, ypos + CollideRect.top + rand()%4 + 10 - 16, WATERFLUSH);
        }
    }
#endif //0
    //DKS - New version of above with bounds checks, divisions converted to multiplies:
    {
        float tmp_y = ypos + (CollideRect.top + 10);
        int tile_y = tmp_y * (1.0f/TILESIZE_Y);
        if (tile_y >= 0 && tile_y < levelsize_y) {
            for (int i = int(xpos) + CollideRect.left + 5; i < int(xpos) + CollideRect.right - 5; i+=2) {
                float tmp_x = i;
                int tile_x = float(i) * (1.0f/TILESIZE_X);
                if (tile_x < 0) continue;
                else if (tile_x >= levelsize_x) break;

                if (TileEngine.TileAt(tile_x, tile_y).Block & BLOCKWERT_WASSERFALL) {
                    if (rand()%50 == 0)
                        PartikelSystem.PushPartikel(tmp_x + rand()%4, tmp_y + rand()%4, WASSERTROPFEN);

                    if (rand()%200 == 0)
                        PartikelSystem.PushPartikel(tmp_x + rand()%4, tmp_y + rand()%4, WATERFUNKE);

                    if (rand()%200 == 0)
                        PartikelSystem.PushPartikel(tmp_x + (rand()%4 - 16), tmp_y + (rand()%4 - 16), WATERFLUSH);
                }
            }
        }
    }

    // Alte Position f�r Kollisionsabfrage sichern
    xposold = xpos;
    yposold = ypos;

    // Auf Fliessband ?
    if ((bo & BLOCKWERT_FLIESSBANDL ||
            bu & BLOCKWERT_FLIESSBANDL) &&
            !(bl & BLOCKWERT_WAND))
        xpos -= 11.0f SYNC;

    if ((bo & BLOCKWERT_FLIESSBANDR ||
            bu & BLOCKWERT_FLIESSBANDR) &&
            !(br & BLOCKWERT_WAND))
        xpos += 11.0f SYNC;

    //---------------------------------------------------------------------------
    // Spieler im normalen Modus, also kein Rad und nicht auf dem FlugSack reiten
    //---------------------------------------------------------------------------

    if (Handlung != RADELN		&&
            Handlung != RADELN_FALL &&
            Handlung != SACKREITEN  &&
            Handlung != SURFEN      &&
            Handlung != SURFENCROUCH&&
            Handlung != SURFENJUMP  &&
            Handlung != DREHEN)
    {
        // Stehen animieren?
        if (Handlung == STEHEN)
        {
            AnimCount += 1.0f SYNC;

            if (AnimCount > 1.0f)
            {
                AnimCount -= 1.0f;
                AnimPhase++;

                if (AnimPhase >= (FRAMES_IDLE * 3) + 44 - 1)
                    AnimPhase = 0;
            }
        }

        // Langsam die Rad-Energie wieder auff�llen ?
        if (Armour < 0.0f)
            Armour = 0.0f;

        Armour += 0.2f SYNC;

        // Oder schneller?
        if (Handlung == STEHEN ||
                Handlung == SCHIESSEN_O)
            Armour += 1.0f SYNC;

        if (Armour > MAX_ARMOUR)
            Armour = MAX_ARMOUR;

        // Richtung umkehren wenn an die Decke gestossen ?
        if(bo & BLOCKWERT_WAND ||
                (TileEngine.Zustand == ZUSTAND_LOCKED &&
                 ypos <= (float)TileEngine.YOffset))
        {
            if (yspeed < 0.0f)
                yspeed = 0.0f;
            JumpAdd = PLAYER_JUMPADDSPEED;
        }

        // Nach Links laufen/springen oder blitzen
        if (Aktion[AKTION_LINKS]  &&	// Links gedr�ckt ?
                !Aktion[AKTION_RECHTS])	 	// und Rechts nicht ?
        {
            if (Handlung == BLITZEN)		// Blitzen und dabei den Blitz bewegen ?
            {
                if (BlitzStart >= PLAYER_BLITZ_START)	// Bewegen schon m�glich ?
                    BlitzWinkel -= 20 SYNC;
            }
            else if (Handlung == BEAMLADEN)		// Rundum bewegen und den Beam aufladen ?
            {
                BlitzWinkel -= 20 SYNC;
            }
            else if(!(bl & BLOCKWERT_WAND))	// Keine Wand im Weg ?
            {
                Blickrichtung = LINKS;					// nach links kucken
                if (Handlung == STEHEN)					// Aus dem Stehen heraus
                    //Aktion[AKTION_OBEN]  == false &&	// und nicht nach oben zielen ?
                    //Aktion[AKTION_UNTEN] == false)		// und nicht nach unten zielen ?
                    AnimPhase = 0;						// das laufen starten

                if (Handlung != SPRINGEN &&				// Nicht in der Luft
                        Handlung != DUCKEN)					// und nicht ducken ?
                    //Aktion[AKTION_OBEN]  == false &&	// und nicht nach oben zielen ?
                    //Aktion[AKTION_UNTEN] == false)		// und nicht nach unten zielen ?
                    Handlung  = LAUFEN;					// Dann laufen wir =)

                if (Handlung != DUCKEN)
                {
                    if (InLiquid == false)
                    {
                        // auf Eis?
                        if (bu & BLOCKWERT_EIS)
                        {
                            xspeed -= PLAYER_ICESPEED SYNC;

                            if (xspeed < -PLAYER_MOVESPEED)
                                xspeed = -PLAYER_MOVESPEED;
                        }
                        else

                            // oder normal
                            xspeed = -PLAYER_MOVESPEED;
                    }
                    else
                        xspeed = -PLAYER_MOVESPEED*2/3;	// im Wasser langsamer

                    if (GegnerDran)
                        xspeed *= 0.5f;
                }

                if (Handlung != SPRINGEN &&
                        Aktion[AKTION_OBEN]  &&
                        WalkLock == true)

                    xspeed = 0.0f;
            }
        }
        // Nach Rechts laufen/springen oder blitzen
        if (Aktion[AKTION_RECHTS] &&		// Rechts gedr�ckt ?
                !Aktion[AKTION_LINKS])			// und Links nicht ?
        {
            if (Handlung == BLITZEN)			// Blitzen und dabei den Blitz bewegen ?
            {
                if (BlitzStart >= PLAYER_BLITZ_START)	// Bewegen schon m�glich ?
                    BlitzWinkel += 20 SYNC;
            }
            else if (Handlung == BEAMLADEN)		// Rundum bewegen und den Beam aufladen ?
            {
                BlitzWinkel += 20 SYNC;
            }
            else if(br | BLOCKWERT_WAND)	// Keine Wand im Weg ?
            {
                Blickrichtung = RECHTS;					// nach rechts kucken
                if (Handlung == STEHEN)					// Aus dem Stehen heraus
                    //Aktion[AKTION_OBEN]  == false &&	// nicht nach oben zielen ?
                    //Aktion[AKTION_UNTEN] == false)		// und nicht nach unten zielen ?
                    AnimPhase = 0;						// das laufen starten

                if (Handlung != SPRINGEN &&				// Nicht in der Luft
                        Handlung != DUCKEN )				// und nicht ducken ?
                    //Aktion[AKTION_OBEN]  == false &&		// nicht nach oben zielen ?
                    //Aktion[AKTION_UNTEN] == false)		// und nicht nach unten zielen ?
                    Handlung  = LAUFEN;					// Dann laufen wir =)

                if (Handlung != DUCKEN)
                {
                    if (InLiquid == false)
                    {
                        // auf Eis?
                        if (bu & BLOCKWERT_EIS)
                        {
                            xspeed += PLAYER_ICESPEED SYNC;

                            if (xspeed > PLAYER_MOVESPEED)
                                xspeed = PLAYER_MOVESPEED;
                        }
                        else

                            // oder normal
                            xspeed = PLAYER_MOVESPEED;
                    }
                    else
                        xspeed = PLAYER_MOVESPEED*2/3;	// im Wasser langsamer

                    if (GegnerDran)
                        xspeed *= 0.5f;
                }

                if (Handlung != SPRINGEN &&
                        Aktion[AKTION_OBEN]  &&
                        WalkLock == true)
                    xspeed = 0.0f;
            }
        }

        // Wand im Weg ? Dann stehenbleiben
        if(((Aktion[AKTION_LINKS]  && (bl & BLOCKWERT_WAND)) ||
                (Aktion[AKTION_RECHTS] && (br & BLOCKWERT_WAND))) &&
                !(bu & BLOCKWERT_SCHRAEGE_L) &&
                !(bu & BLOCKWERT_SCHRAEGE_R))

        {
            if (Handlung == LAUFEN)
            {
                Handlung  = STEHEN;
                AnimPhase = 0;
            }
            xspeed	  = 0.0f;
        }

        // Rechts/Links gedr�ckt ? stehenbleiben, da man ja nich r�ckw�rts UND vorw�rts gehen kann =)
        if(Aktion[AKTION_LINKS] && Aktion[AKTION_RECHTS])
        {
            if ((Handlung != SPRINGEN || JumpPossible == true) &&
                    Handlung != BLITZEN	&&
                    Handlung != BEAMLADEN)
            {
                AnimPhase = 0;
                Handlung  = STEHEN;
            }
        }

        // Bei keiner Tastatureingabe steht der Spieler ruhig da
        if (!Aktion[AKTION_LINKS]	&&
                !Aktion[AKTION_RECHTS]	&&
                Handlung != SPRINGEN	&&
                Handlung != BLITZEN		&&
                Handlung != PISSEN		&&
                Handlung != BEAMLADEN   &&
                ShotDelay <= 0.5f)
        {
            if (Handlung != SPRINGEN)
                Handlung  = STEHEN;
            //AnimPhase = 0;
        }

        // im Sumpf langsam einsinken
        uint32_t busumpf = TileEngine.BlockUnten(xpos, ypos, xposold, yposold, CollideRect, yspeed >= 0.0f);
        if (busumpf & BLOCKWERT_SUMPF ||
                bl & BLOCKWERT_SUMPF ||
                br & BLOCKWERT_SUMPF)
        {
            if (!Aktion[AKTION_JUMP])
                JumpPossible = true;

            if (!(busumpf &  BLOCKWERT_WAND) &&
                    yspeed >= 0.0f)
                ypos += 4.0f SYNC;

            if (Handlung == SPRINGEN &&
                    yspeed >= 0.0f)
                Handlung = LAUFEN;
        }

        // Springen (nur, wenn nicht schon in der Luft)
        if (Aktion[AKTION_JUMP] &&
                JumpPossible == true)
        {
            int save = AnimPhase;				// F�r Sumpf

            SoundManager.StopWave (SOUND_BEAMLOAD + SoundOff);
            JumpPossible = false;
            AnimPhase = 2;						// das springen starten
            Handlung  = SPRINGEN;
            JumpStart = ypos;
            yspeed    = -PLAYER_MAXJUMPSPEED;

            JumpAdd	  =  0.0f;

            // Gegner dran festgebissen? Dann kann man nicht so hoch springen
            if (GegnerDran)
            {
                yspeed *= 0.5f;
                JumpAdd = PLAYER_JUMPADDSPEED;
            }

            if (AufPlattform != NULL)
            {
                GegnerMushroom *pTemp;

                if (AufPlattform->GegnerArt == MUSHROOM)
                {
                    pTemp = (GegnerMushroom*)(AufPlattform);
                    pTemp->PlayerJumps(this);
                }

                AufPlattform = NULL;
            }

            // Blubbersound noch beim Springen im Wasser
            //DKS - Added function WaveIsPlaying() to SoundManagerClass:
            //if (InLiquid == true &&
            //        SoundManager.its_Sounds[SOUND_DIVE]->isPlaying == false)
            //    SoundManager.PlayWave(100, rand()%255, 8000 + rand()%4000, SOUND_DIVE);
            if (InLiquid == true && !SoundManager.WaveIsPlaying(SOUND_DIVE))
                SoundManager.PlayWave(100, rand()%255, 8000 + rand()%4000, SOUND_DIVE);

            // Im Sumpf? Dann ein paar Pixel anheben und wieder runterfallen
            if (bu & BLOCKWERT_SUMPF)
            {
                ypos -= 15.0f;

                // noch nicht aus dem Sumpf draussen? Dann gleich wieder runterfallen
                //DKS - Rewrote BlockSlopes function to only take the parameters it uses:
                //bu = TileEngine.BlockSlopes     (xpos, ypos, xposold, yposold, CollideRect, yspeed);
                bu = TileEngine.BlockSlopes     (xpos, ypos, CollideRect, yspeed);
                if (bu & BLOCKWERT_SUMPF)
                {
                    //yspeed = 0.0f;
                    JumpPossible = false;
                    AnimPhase = save;
                    //Handlung = STEHEN;
                }
            }
        }

        // Ist ein Sprung m�glich ? (Wenn der Spieler wieder auf den Boden aufkommt)
        if ((bu & BLOCKWERT_SUMPF)		||
                (bu & BLOCKWERT_WAND)		||
                (bu & BLOCKWERT_PLATTFORM)  ||
                (bu & BLOCKWERT_SCHRAEGE_L)  ||
                (bu & BLOCKWERT_SCHRAEGE_R)  ||
                AufPlattform != NULL)
        {
            if (!(bu & BLOCKWERT_SCHRAEGE_L) &&
                    !(bu & BLOCKWERT_SCHRAEGE_R))
            {
                if (yspeed > 0.0f)
                {
                    // Beim Blitz weiterblitzen, ansonsten auf "STEHEN" schalten
                    //
                    if (Handlung != BLITZEN)
                    {
                        Handlung = STEHEN;
                        AnimPhase = 0;
                    }

                    // stehen bleiben
                    yspeed = 0.0f;
                    SoundManager.PlayWave(100, 128, 11025, SOUND_LANDEN);
                    PartikelSystem.PushPartikel(xpos+20, ypos+60, SMOKE);
                }
            }
            else
                //DKS - Rewrote BlockSlopes function to only take the parameters it uses:
                //bu = TileEngine.BlockSlopes     (xpos, ypos, xposold, yposold, CollideRect, yspeed);
                bu = TileEngine.BlockSlopes     (xpos, ypos, CollideRect, yspeed);

            if (JumpPossible		== false &&
                    Aktion[AKTION_JUMP] == false &&
                    yspeed >= 0.0f)
                JumpPossible = true;
        }

        // Kein Block unter dem Spieler und kein Lift? Dann f�llt er runter
        if (AufPlattform == NULL     &&
                Handlung != SPRINGEN     &&
                yspeed >= 0.0f			 &&
                (!(bu & BLOCKWERT_WAND)) &&
                (!(bu & BLOCKWERT_PLATTFORM)) &&
                (!(bu & BLOCKWERT_SUMPF)) &&
                (!(bl & BLOCKWERT_SCHRAEGE_L)) &&
                (!(bl & BLOCKWERT_SCHRAEGE_R)) &&
                (!(br & BLOCKWERT_SCHRAEGE_L)) &&
                (!(br & BLOCKWERT_SCHRAEGE_R)) &&

                (!(bu & BLOCKWERT_SCHRAEGE_L)) &&
                (!(bu & BLOCKWERT_SCHRAEGE_R)))
        {
            JumpPossible = false;

            if (Handlung != BLITZEN)
            {
                Handlung  = SPRINGEN;
                AnimPhase = 0;
                //yspeed    = 0.5f;
            }

            if ((Handlung == BLITZEN &&
                    yspeed == 0.0f))
                yspeed = 0.5f;

            JumpAdd   = PLAYER_JUMPADDSPEED;
        }

        // Am Boden und nicht im Sumpf oder Damage-Tile? Dann die Position sichern
        if (Handlung != SPRINGEN &&
                AufPlattform == NULL &&
                !(bo & BLOCKWERT_SUMPF) &&
                !(bu & BLOCKWERT_SUMPF) &&
                !(bl & BLOCKWERT_SUMPF) &&
                !(br & BLOCKWERT_SUMPF) &&
                !(bo & BLOCKWERT_SCHADEN) &&
                !(bu & BLOCKWERT_SCHADEN) &&
                !(bl & BLOCKWERT_SCHADEN) &&
                !(br & BLOCKWERT_SCHADEN))
        {
            JumpxSave = xpos;
            JumpySave = ypos;
        }

        // Vom Ducken aufstehen
        // nur, wenn keine Decke �ber dem Spieler (wenn er grad mit ner Plattform wo drunter durchf�hrt z.B.)
        if (Handlung == DUCKEN)
        {
            float ypos2 = ypos - 48.0f;
            uint32_t bo2 = TileEngine.BlockOben  (xpos, ypos2, xposold, yposold, CollideRect, true);

            if (!(bo  & BLOCKWERT_WAND) &&
                    !(bo2 & BLOCKWERT_WAND))
                Handlung = STEHEN;
        }

        CollideRect.top = 12;	// Blockierung wieder wie im Stehen

        // oder gleich wieder geduckt bleiben ?
        if (Aktion[AKTION_DUCKEN] == true  &&
                //Aktion[AKTION_LINKS]  == false &&
                //Aktion[AKTION_RECHTS] == false &&
                JumpPossible == true		&&
                Handlung != BEAMLADEN		&&
                Handlung != BLITZEN)
        {
            Handlung = DUCKEN;
            CollideRect.top = 32;	// Im Ducken liegt die obere Blockgrenze tiefer
        }

        // Blitz schiessen ? (nur, wenn am Boden)
        if (Aktion[AKTION_BLITZ] && JumpPossible == true)
        {
            // Blitz aufladen?
            if (Aktion[AKTION_SHOOT] && Handlung != BEAMLADEN)
            {
                SoundManager.PlayWave (100, 128, 9000, SOUND_BEAMLOAD + SoundOff);

                DirectInput.Joysticks[JoystickIndex].ForceFeedbackEffect(FFE_BLITZ);

                Handlung   = BEAMLADEN;
                BlitzStart = 5.0f;
                AnimPhase  = 0;
                if (Blickrichtung == LINKS)			// Blitz je nach Blickrichtung neu
                    BlitzWinkel = 270;				// geradeaus richten
                else
                    BlitzWinkel = 90;

                if (Aktion[AKTION_OBEN])
                {
                    Blickrichtung *= -1;
                    BlitzWinkel = 0;
                }
            }

            else

                if (Handlung != BLITZEN && Handlung != BEAMLADEN)			// Blitz wird erst gestartet ?
                {
                    BlitzStart = 0.0f;
                    BeamCount  = 0.1f;
                    AnimPhase = 0;
                    Handlung  = BLITZEN;
                    changecount = 0.0f;

                    // Blitz Startsound ausgeben
                    SoundManager.PlayWave(100, 128, rand()%500+18025, SOUND_BLITZSTART + SoundOff);

                    if (Blickrichtung == LINKS)			// Blitz je nach Blickrichtung neu
                        BlitzWinkel = 270;				// geradeaus richten
                    else
                        BlitzWinkel = 90;
                }
        }

        if (Aktion[AKTION_BLITZ] == false && Handlung == BLITZEN)
        {
            // Im Fallen? Dann in Sprunganimation wechseln
            //
            if (yspeed > 0.0f)
            {
                Handlung = SPRINGEN;
            }

            // Im Stehen? Dann je nach aktueller Blickrichtung
            // richtig hinstellen
            //
            else
            {
                if (AnimPhase >= 9 &&
                        AnimPhase <= 18)
                    Blickrichtung *= -1;

                Handlung = STEHEN;
            }

            AnimPhase = 0;
        }

        // In das Rad verwandeln ?
        if (JumpPossible			 ==	true &&
                Aktion[AKTION_POWERLINE] == true &&
                Aktion[AKTION_DUCKEN]    == true &&
                Armour > 5.0f)
        {
            Handlung  = RADELN;
            AnimPhase = 0;
            AnimCount = 0.0f;
            CollideRect.left		= 20;	// Neue Abmessungen f�r das Rad
            CollideRect.top			= 40;
            CollideRect.right		= 50;
            CollideRect.bottom		= 79;
            PowerLinePossible		= false;
        }

        // Geloopten Blitzsound ausgeben
        if (Handlung == BLITZEN)
        {
            // Blitz Sound abspielen, wenn er nicht schon abgespielt wird
            //DKS - Added function WaveIsPlaying() to SoundManagerClass:
            //if (BlitzStart >= PLAYER_BLITZ_START &&
            //        true == SoundManager.InitSuccessfull &&
            //        SoundManager.its_Sounds[SOUND_BLITZ + SoundOff]->isPlaying == false)
            if (BlitzStart >= PLAYER_BLITZ_START &&
                    !SoundManager.WaveIsPlaying(SOUND_BLITZ + SoundOff))
            {
                SoundManager.PlayWave(100, 128, 11025, SOUND_BLITZ + SoundOff);
                DirectInput.Joysticks[JoystickIndex].ForceFeedbackEffect(FFE_BLITZ);
            }
        }
        //DKS - Added function WaveIsPlaying() to SoundManagerClass:
        //else if (true == SoundManager.InitSuccessfull &&
        //         SoundManager.its_Sounds[SOUND_BLITZ + SoundOff]->isPlaying == true)
        else if (SoundManager.WaveIsPlaying(SOUND_BLITZ + SoundOff))
        {
            SoundManager.StopWave(SOUND_BLITZ + SoundOff);
            SoundManager.PlayWave(100, 128, rand()%1000+11025, SOUND_BLITZENDE + SoundOff);
            DirectInput.Joysticks[JoystickIndex].StopForceFeedbackEffect(FFE_BLITZ);
        }

        // Spieler nach BlitzRichtung ausrichten
        if (Handlung == BLITZEN)
        {
            // Verz�gerung beim Blitzen
            if (BlitzStart < PLAYER_BLITZ_START)
                BlitzStart += 1.0f SYNC;
            else
            {
                float Winkel;

                Winkel = BlitzWinkel - 270;		// 270� beim nach links kucken = Animphase 0
                if (Winkel < 0.0f)
                    Winkel += 360.0f;

                // Je nach Richtung Winkel umdrehen
                if (Blickrichtung == LINKS)
                    Winkel = 360.0f-Winkel;
                else
                    Winkel = 180 + Winkel;

                AnimPhase = (int)((Winkel + 0.0f) / 10.0f) % FRAMES_SURROUND;
            }
        }

        // Beam Laden und richtige Animationsphase setzen
        //
        if (Handlung == BEAMLADEN)
        {
            //PullItems();  //DKS - was already commented out

            //DKS - Added check for NULLness and DirectX, since SDL port doesn't support SetFrequency.
            //      Also added function SetWaveFrequency()
#if defined(PLATFORM_DIRECTX)
            int Freq = 9000 + (int)(BlitzStart / 20.0f * 2000);
            //if (SoundManager.its_Sounds[SOUND_BEAMLOAD + SoundOff] != NULL)
            //    SOUND_SetFrequency(SoundManager.its_Sounds[SOUND_BEAMLOAD + SoundOff]->Channel, Freq);
            SetWaveFrequency((SOUND_BEAMLOAD + SoundOff), Freq);
#endif

            // Beam aufladen. Je l�nger der Blitz desto schneller l�dt der Beam
            if (BlitzStart < PLAYER_BEAM_MAX)
                BlitzStart += CurrentWeaponLevel[3] * 1.0f SYNC;

            float Winkel;

            Winkel = BlitzWinkel - 270;		// 270� beim nach links kucken = Animphase 0
            if (Winkel < 0.0f)
                Winkel += 360.0f;

            // Je nach Richtung Winkel umdrehen
            if (Blickrichtung == LINKS)
                Winkel = 360.0f-Winkel;
            else
                Winkel = 180 + Winkel;

            AnimPhase = (int)((Winkel + 0.0f) / 10.0f) % FRAMES_SURROUND;

            // Beam abfeuern
            if (!Aktion[AKTION_BLITZ] || !Aktion[AKTION_SHOOT])
            {
                Handlung = STEHEN;
                SoundManager.StopWave(SOUND_BEAMLOAD + SoundOff);
                DirectInput.Joysticks[JoystickIndex].StopForceFeedbackEffect(FFE_BLITZ);

                if (BlitzStart >= 20.0f)
                {
                    Projectiles.PushBlitzBeam(int(BlitzStart), BlitzWinkel, this);
                    SoundManager.PlayWave(100, 128, 7000, SOUND_BLITZENDE + SoundOff);
                    SoundManager.PlayWave(100, 128, 10000 + rand()%2000, SOUND_BLITZENDE + SoundOff);
                    SoundManager.PlayWave(100, 128, 10000, SOUND_EXPLOSION3);
                }

                // je nach Ausrichtung der Waffe in die richtige Richtung kucken
                //
                if (AnimPhase >= 12 &&
                        AnimPhase <= 24)
                    Blickrichtung *= -1;
            }
        }

        // Piss-Animation
        //
        if (Handlung == PISSEN)
        {
            AnimCount += 1.0f SYNC;

            if (AnimCount > 1.0f)
            {
                AnimCount = 0.0f;
                AnimPhase++;

                if (AnimPhase > 2)
                    AnimCount = -1.0f;

                if (AnimPhase == 11)
                    AnimCount = -2.0f;
            }

            if (AnimPhase == 16)
                AnimPhase = 3;
        }
    }

    else

        //--------------------
        // Als Rad rumkullern
        //--------------------

        if (Handlung == RADELN ||
                Handlung == RADELN_FALL)

        {
            // Rad-Energie abziehen
            if (WheelMode == false)
                Armour -= float(2.5 SYNC);

            if (Armour < 0.0f)
                Armour = 0.0f;

            // Powerline schiessen ?
            if (Aktion[AKTION_POWERLINE] == true  &&
                    PowerLinePossible		 == true  && PowerLines > 0)
            {
                SoundManager.PlayWave(100, 128, 11025, SOUND_POWERLINE);
                PowerLinePossible = false;			// Taste einrasten
                PowerLines--;						// Powerlines verringern

                for (int i=-1; i<25; i++)			// Powerlines schiessen
                {
                    Projectiles.PushProjectile (xpos+20, float(int(TileEngine.YOffset/20)*20 + i*20), POWERLINE, this);
                    Projectiles.PushProjectile (xpos+20, float(int(TileEngine.YOffset/20)*20 + i*20), POWERLINE2, this);
                }
            }

            // SmartBomb abfeuern ?
            if (Aktion[AKTION_SMARTBOMB] == true  &&
                    PowerLinePossible == true && SmartBombs > 0)
            {
                SoundManager.PlayWave(100, 128, 8000, SOUND_POWERLINE);
                PowerLinePossible = false;			// Taste einrasten
                SmartBombs--;						// SmartBombs verringern

                Projectiles.PushProjectile (xpos+40-32, ypos+55-32, SMARTBOMB, this);
            }

            // Taste losgelassen ? Dann k�nnen wir wieder eine Powerline schiessen
            if (Aktion[AKTION_POWERLINE] == false &&
                    Aktion[AKTION_SMARTBOMB] == false)
                PowerLinePossible = true;

            // Ja nach Blickrichtung in die richtige Richtung scrollen
            if (Blickrichtung == LINKS && (!(bl & BLOCKWERT_WAND)))
            {
                if (InLiquid == false)
                    xspeed = -40.0f;
                else
                    xspeed = -30.0f;
            }
            else if (Blickrichtung == RECHTS && (!(br & BLOCKWERT_WAND)))
            {
                if (InLiquid == false)
                    xspeed = 40.0f;
                else
                    xspeed = 30.0f;
            }

            if (Aktion[AKTION_LINKS] == true &&		// Nach Links rollen ?
                    (Handlung == RADELN   ||			// Nur wenn man Boden unter den F��en hat
                     AufPlattform != NULL ||
                     bu & BLOCKWERT_WAND  ||
                     bu & BLOCKWERT_PLATTFORM))
                Blickrichtung = LINKS;

            if (Aktion[AKTION_RECHTS] == true &&	// Nach Rechts rollen ?
                    (Handlung == RADELN   ||			// Nur wenn man Boden unter den F��en hat
                     AufPlattform != NULL ||
                     bu & BLOCKWERT_WAND  ||
                     bu & BLOCKWERT_PLATTFORM))
                Blickrichtung = RECHTS;

            if (Handlung != RADELN_FALL &&
                    (!(bu & BLOCKWERT_SCHRAEGE_R)) &&
                    (!(bu & BLOCKWERT_SCHRAEGE_L)) &&
                    (!(bu & BLOCKWERT_WAND)) &&
                    (!(bu & BLOCKWERT_PLATTFORM)))
            {
                Handlung  =	RADELN_FALL;
                yspeed    = 0.5f;
                JumpAdd   = PLAYER_JUMPADDSPEED;
            }

            // Wieder am Boden aufgekommen ?
            if (Handlung == RADELN_FALL &&
                    ((AufPlattform != NULL) ||
                     (bu & BLOCKWERT_WAND) ||
                     (bu & BLOCKWERT_PLATTFORM)))
            {
                if (yspeed > 2.0f)
                    PartikelSystem.PushPartikel(xpos+20, ypos+60, SMOKE);

                if (yspeed > 0.0f)
                {
                    yspeed = -yspeed*2/3;				// Abhopfen

                    if (yspeed > -5.0f)					// oder ggf wieder
                    {
                        // Normal weiterkullern
                        yspeed    = 0.0f;
                        JumpAdd   = 0.0f;
                        Handlung  = RADELN;
                        TileEngine.BlockUnten(xpos, ypos, xposold, yposold, CollideRect, true);
                    }
                    else
                    {
                        AufPlattform = NULL;
                        SoundManager.PlayWave(100, 128, 11025, SOUND_LANDEN);
                    }
                }
            }

            // An die Wand gestossen ? Dann Richtung umkehren
            if (!(bu & BLOCKWERT_SCHRAEGE_R) &&
                    !(bu & BLOCKWERT_SCHRAEGE_L))
            {
                if (Blickrichtung == RECHTS && (br & BLOCKWERT_WAND))
                    Blickrichtung = LINKS;

                if (Blickrichtung == LINKS && (bl & BLOCKWERT_WAND))
                    Blickrichtung = RECHTS;
            }

            // Aufh�ren zu kullern und zur�ckverwandeln, wenn man springt oder keine Energie mehr hat
            // funktiomiert nur, wenn �ber einem keine Wand ist. Dabei wird noch ein Teil mehr gecheckt, als
            // das Rad hoch ist, damit man den Zustand pr�ft, als wenn der Spieler wieder stehen w�rde

            float ypos2 = ypos - 20.0f;
            uint32_t bo2 = TileEngine.BlockOben  (xpos, ypos2, xposold, yposold, CollideRect, true);

            if ((Armour <= 0.0f ||
                    Aktion[AKTION_JUMP])   &&
                    !(bo  & BLOCKWERT_WAND) &&
                    !(bo2 & BLOCKWERT_WAND))
            {
                AnimPhase = 0;

                // Am Boden zur�ckverwandelt ?
                if (Armour <= 0.0f &&
                        ((bu & BLOCKWERT_WAND) ||
                         (bu & BLOCKWERT_PLATTFORM)))
                    Handlung  = STEHEN;

                // oder in der Luft bzw durch Sprung
                else
                    Handlung  = SPRINGEN;	// Nein, dann in der Luft

                CollideRect.left		= 20;
                CollideRect.top			= 12;
                CollideRect.right		= 50;
                CollideRect.bottom		= 79;
                if (Armour < 0.0f)
                    Armour = 0.0f;
            }
        }

        else

            //------------------------
            // Auf dem FlugSack reiten
            //------------------------

            if (Handlung == SACKREITEN ||
                    Handlung == DREHEN)
            {
                // Spieler und Level bewegen (wenn nicht beim Endboss bzw wenn Level scrollbar)
                // Nur, wenn alle Spieler auf einem Flugsack sitzen
                //
                bool BeideFrei = true;

                for (int p = 0; p < NUMPLAYERS; p++)
                    if (Player[p].FesteAktion > -1)
                        BeideFrei = false;

                if (FlugsackFliesFree == false &&
                        Riding() &&
                        BeideFrei == true)
                    ypos -= PLAYER_FLUGSACKSPEED SYNC;

                JumpySave = ypos;
                JumpxSave = xpos;

                // unten rausgeflogen ? Dann wieder zur�cksetzen
                if (ypos > TileEngine.YOffset + 475.0f)
                {
                    SoundManager.PlayWave (100, 128, 11025, SOUND_EXPLOSION2);
                    xpos    = JumpxSave;	// Alte Position wieder herstellen, wenn der
                    ypos    = JumpySave;	// der Spieler zB einen Abgrund runterfiel
                    xposold = JumpxSave;
                    yposold = JumpySave;
                    Energy  = 0.0f;
                }

                // An der Decke ansto�en
                if (ypos < TileEngine.YOffset &&
                        BeideFrei)
                {
                    if (yadd < 0.0f)
                        yadd *= -1.0f;
                }

                // W�nde checken
                if (bl & BLOCKWERT_WAND) if (xadd < 0.0f) xadd = -xadd / 2.0f;
                if (br & BLOCKWERT_WAND) if (xadd > 0.0f) xadd = -xadd / 2.0f;
                if (bo & BLOCKWERT_WAND) if (yadd < 0.0f) yadd = -yadd / 2.0f;
                if (bu & BLOCKWERT_WAND) if (yadd > 0.0f) yadd = -yadd / 2.0f;

                // Rauch am Flugsack erzeugen
                SmokeCount -= 1.0f SYNC;
                if (SmokeCount < 0.0f)
                {
                    SmokeCount += 0.1f;

                    // Beim Reiten
                    if (Handlung == SACKREITEN)
                    {
                        if (Blickrichtung == LINKS)
                            PartikelSystem.PushPartikel(xpos + 72, ypos + 100, FLUGSACKSMOKE2);
                        else
                            PartikelSystem.PushPartikel(xpos - 2,  ypos + 100, FLUGSACKSMOKE);
                    }

                    // oder beim Drehen
                    if (Handlung == DREHEN)
                    {
                        if (Blickrichtung == LINKS)
                        {
                            PartikelSystem.PushPartikel(xpos + 70 - AnimPhase * 10, ypos + 100, FLUGSACKSMOKE2);
                            PartikelSystem.PushPartikel(xpos + 87 - AnimPhase * 10, ypos + 100, FLUGSACKSMOKE);
                        }
                        else
                        {
                            PartikelSystem.PushPartikel(xpos + 4  + (AnimPhase-10) * 10,  ypos + 100, FLUGSACKSMOKE);
                            PartikelSystem.PushPartikel(xpos - 20 + (AnimPhase-10) * 10,  ypos + 100, FLUGSACKSMOKE2);
                        }
                    }
                }

                // Umdrehen
                if (Handlung == DREHEN)
                {
                    AnimCount += 1.2f SYNC;

                    if (AnimCount > 0.6f)
                    {
                        AnimCount -= 0.6f;
                        AnimPhase++;

                        // Zuende gedreht ?
                        if (AnimPhase == AnimEnde)
                        {
                            AnimEnde = 0;
                            Handlung = SACKREITEN;

                            if (Blickrichtung == RECHTS)
                            {
                                AnimPhase = 0;
                                Blickrichtung = LINKS;
                            }
                            else
                            {
                                AnimPhase = 10;
                                Blickrichtung = RECHTS;
                            }
                        }
                    }
                }

                // Links fliegen
                if (Aktion[AKTION_LINKS] == true && Aktion[AKTION_RECHTS]== false)
                {
                    xadd -= 10.0f SYNC;

                    // Drehen ?
                    if (Aktion[AKTION_SHOOT] == false &&
                            Handlung      == SACKREITEN	  &&
                            Blickrichtung == RECHTS)
                    {
                        Handlung  = DREHEN;
                        AnimEnde  = 19;
                        AnimCount = 0.0f;
                        AnimPhase = 10;
                    }
                }

                // Rechts fliegen
                if (Aktion[AKTION_RECHTS] == true && Aktion[AKTION_LINKS]== false)
                {
                    xadd += 10.0f SYNC;

                    // Drehen ?
                    if (Aktion[AKTION_SHOOT] == false &&
                            Handlung      == SACKREITEN   &&
                            Blickrichtung == LINKS)
                    {
                        Handlung  = DREHEN;
                        AnimEnde  = 9;
                        AnimCount = 0.0f;
                        AnimPhase = 0;
                    }
                }

                // Hoch fliegen
                if ((Aktion[AKTION_OBEN] == true ||
                        Aktion[AKTION_JUMP] == true) &&
                        Aktion[AKTION_DUCKEN]== false)
                    yadd -= 10.0f SYNC;

                // Runter fliegen
                if (Aktion[AKTION_DUCKEN] == true &&
                        Aktion[AKTION_OBEN]== false)
                    yadd += 10.0f SYNC;

                // Bewegung abbremsen
                if (!Aktion[AKTION_LINKS] &&
                        !Aktion[AKTION_RECHTS])
                {
                    if (xadd < 0.0f)
                    {
                        xadd += 4.0f SYNC;
                        if (xadd > 0.0f)
                            xadd = 0.0f;
                    }

                    if (xadd > 0.0f)
                    {
                        xadd -= 4.0f SYNC;
                        if (xadd < 0.0f)
                            xadd = 0.0f;
                    }
                }

                if (!Aktion[AKTION_OBEN] &&
                        !Aktion[AKTION_DUCKEN])
                {
                    if (yadd < 0.0f)
                    {
                        yadd += 4.0f SYNC;
                        if (yadd > 0.0f)
                            yadd = 0.0f;
                    }

                    if (yadd > 0.0f)
                    {
                        yadd -= 4.0f SYNC;
                        if (yadd < 0.0f)
                            yadd = 0.0f;
                    }
                }

                // Geschwindigkeitsgrenzen checken
                if (xadd >  25.0f) xadd =  25.0f;
                if (xadd < -25.0f) xadd = -25.0f;
                if (yadd >  25.0f) yadd =  25.0f;
                if (yadd < -25.0f) yadd = -25.0f;

                xspeed = xadd;
                ypos += yadd SYNC;

                // Wendepunkt erreicht? Dann automatisch abspringen
                float ytemp = ypos - 1;
                bu = TileEngine.BlockUntenNormal(xpos, ypos, xposold, ytemp, CollideRect);
                if (bu & BLOCKWERT_WENDEPUNKT)
                {
                    Handlung = SPRINGEN;
                    AnimPhase = 0;
                    yspeed = -PLAYER_MAXJUMPSPEED;
                    JumpAdd = PLAYER_JUMPADDSPEED;
                    AufPlattform = NULL;

                    // abst�rzenden Flugsack adden
                    Gegner.PushGegner(xpos, ypos + 20, FLUGSACK, 99, 0, false);

                    FlugsackFliesFree = true;
                    TileEngine.Zustand = ZUSTAND_SCROLLBAR;

                    SoundManager.FadeSong(MUSIC_FLUGSACK, -2.0f, 0, true);
                    SoundManager.FadeSong(MUSIC_STAGEMUSIC, 2.0f, 100, true);
                }
            }

    //-------------------------
    // Auf dem SurfBrett stehen
    //-------------------------

    //DKS - This appears never to have been implemented (no image for it) so disabled it:
#if 0
    if (Handlung == SURFEN     ||
            Handlung == SURFENJUMP ||
            Handlung == SURFENCROUCH)
    {
        static float Antrieb = 0.0f;

        // Beim ersten Draufspringen geht das Board kurz ab
        if (JumpedOnSurfboard == true)
        {
            JumpedOnSurfboard = false;
            xadd = 30.0f;
        }

        // Spieler und Level bewegen (wenn nicht beim Endboss bzw wenn Level scrollbar)
        //
        xpos			     += AutoScrollspeed SYNC;
        TileEngine.XOffset += AutoScrollspeed SYNC;

        if (Handlung == SURFENCROUCH)
            Handlung = SURFEN;

        // Ducken
        if (Aktion[AKTION_DUCKEN] == true  &&
                Aktion[AKTION_LINKS]  == false &&
                Aktion[AKTION_RECHTS] == false &&
                JumpPossible == true           &&
                Handlung != BLITZEN)
        {
            Handlung = SURFENCROUCH;
            CollideRect.top = 32;	// Im Ducken liegt die obere Blockgrenze tiefer
        }

        // Springen (nur, wenn nich schon in der Luft)
        if (Aktion[AKTION_JUMP] && JumpPossible == true)
        {
            Handlung = SURFENJUMP;
        }

        // Links surfen
        if (Aktion[AKTION_LINKS] == true && Aktion[AKTION_RECHTS]== false)
            xadd -= 12.0f SYNC;

        // Rechts surfen
        if (Aktion[AKTION_RECHTS] == true && Aktion[AKTION_LINKS]== false)
        {
            xadd += 12.0f SYNC;
            Antrieb -= 1.0f SYNC;

            // Antriebsflamme erzeugen
            //
            if (Antrieb < 0.0f)
            {
                PartikelSystem.PushPartikel (xpos - 10, ypos + 60 + rand()%6, ROCKETSMOKE);
                Antrieb += 0.1f;
            }
        }

        // Bewegung abbremsen
        if (!Aktion[AKTION_LINKS] && !Aktion[AKTION_RECHTS]) xadd *= 0.99f;

        // Geschwindigkeitsgrenzen checken
        if (xadd >  25.0f) xadd =  25.0f;
        if (xadd < -25.0f) xadd = -25.0f;

        xspeed = xadd;
    }
#endif //0

    //----------------------
    // Powerline schiessen ?
    //----------------------

    if (Aktion[AKTION_POWERLINE] == true  &&
            Aktion[AKTION_DUCKEN]    == false &&
            PowerLinePossible		 == true  && PowerLines > 0)
    {
        SoundManager.PlayWave(100, 128, 11025, SOUND_POWERLINE);
        PowerLinePossible = false;			// Taste einrasten
        PowerLines--;						// Powerlines verringern

        for (int i=-1; i<25; i++)			// Powerlines schiessen
        {
            Projectiles.PushProjectile (xpos+20, float(int(TileEngine.YOffset/20)*20 + i*20), POWERLINE, this);
            Projectiles.PushProjectile (xpos+20, float(int(TileEngine.YOffset/20)*20 + i*20), POWERLINE2, this);
        }
    }

    //---------------------
    // SmartBomb abfeuern ?
    //---------------------

    if (Aktion[AKTION_SMARTBOMB] == true  &&
            PowerLinePossible == true && SmartBombs > 0)
    {
        SoundManager.PlayWave(100, 128, 8000, SOUND_POWERLINE);
        PowerLinePossible = false;			// Taste einrasten
        SmartBombs--;						// SmartBombs verringern

        Projectiles.PushProjectile (xpos+40-32, ypos+35-32, SMARTBOMB, this);
    }

    //--------------------------------------------------------------------
    // Taste losgelassen ? Dann k�nnen wir wieder eine Powerline schiessen
    //--------------------------------------------------------------------

    if (Aktion[AKTION_POWERLINE] == false &&
            Aktion[AKTION_GRANATE]   == false &&
            Aktion[AKTION_SMARTBOMB] == false)
        PowerLinePossible = true;

    // --------------------------------
    // Hoch / runter kucken bzw. zielen
    // --------------------------------

    // nach oben zielen
    if (Aktion[AKTION_OBEN])
    {
        if (Handlung == STEHEN	     ||
                Handlung == SCHIESSEN_O  ||
                Handlung == SCHIESSEN_LO ||
                Handlung == SCHIESSEN_RO ||
                Handlung == LAUFEN)
        {
            if (Aktion[AKTION_LINKS])
            {
                Handlung      = SCHIESSEN_LO;
                Blickrichtung = LINKS;
            }
            else if (Aktion[AKTION_RECHTS])
            {
                Handlung      = SCHIESSEN_RO;
                Blickrichtung = RECHTS;
            }
            else
                Handlung = SCHIESSEN_O;
        }
    }

    if ((!Aktion[AKTION_OBEN]	&&
            !Aktion[AKTION_UNTEN]) ||
            NUMPLAYERS ==2 ||
            TileEngine.Zustand != ZUSTAND_SCROLLBAR)
        look = 0.0f;

    if (TileEngine.Zustand == ZUSTAND_SCROLLBAR	  &&
            NUMPLAYERS < 2 &&
            Handlung != SACKREITEN &&
            Handlung != DREHEN &&
            Handlung != SURFEN &&
            Handlung != SURFENJUMP &&
            Handlung != SURFENCROUCH)
    {
        // Spieler steht am Boden ?
        //

        if (Handlung != SPRINGEN)
        {
            // Hoch scrollen / nach oben zielen
            if (Aktion[AKTION_OBEN])
            {
                look += 1.0f SYNC;

                if (look > 5.0f)
                {
                    if (TileEngine.YOffset > ypos - 400.0f)
                        TileEngine.YOffset -= 19.0f SYNC;
                    else
                        TileEngine.YOffset = ypos - 400.0f;
                }
            }

            // Runter scrollen bzw. runter zielen
            if (Aktion[AKTION_UNTEN])
            {
                look += 1.0f SYNC;

                if (look > 5.0f ||
                        AktionKeyboard[AKTION_UNTEN] != AktionKeyboard[AKTION_DUCKEN])
                {
                    if (TileEngine.YOffset < ypos - 40.0f)
                        TileEngine.YOffset += 19.0f SYNC;
                    else
                        TileEngine.YOffset = ypos - 40.0f;
                }
            }
        }

        else
        {
            if (ypos-TileEngine.YOffset < SCROLL_BORDER_EXTREME_TOP)
                TileEngine.YOffset = ypos - SCROLL_BORDER_EXTREME_TOP;

            if (ypos-TileEngine.YOffset > SCROLL_BORDER_EXTREME_BOTTOM &&
                    yspeed > 0.0f)
                TileEngine.YOffset = ypos - SCROLL_BORDER_EXTREME_BOTTOM;
        }

        // extrem Rand trotzdem nochmal checken
        if (Handlung != SACKREITEN &&
                Handlung != DREHEN)
        {
            if (ypos-TileEngine.YOffset < 40)  TileEngine.YOffset = ypos - 40;
            if (ypos-TileEngine.YOffset > 380) TileEngine.YOffset = ypos - 380;
        }
    }

    // Fahrstuhl hat andere Grenzen
    if (Riding() == false &&
            TileEngine.IsElevatorLevel == true &&
            ypos-TileEngine.YOffset > 300)
        TileEngine.YOffset = ypos - 300;

    // -----------------
    // Spieler animieren
    // -----------------

    // Hat der Spieler ein Schutzschild ?

    if (Shield > 0.0f)
        Shield -= 0.4f SYNC;

    //-----------------------------------------------
    // Energie abziehen

    if ((bo & BLOCKWERT_SCHADEN) ||
            (bu & BLOCKWERT_SCHADEN) ||
            (bl & BLOCKWERT_SCHADEN) ||
            (br & BLOCKWERT_SCHADEN))
    {
        switch (Skill)
        {
        case 0:
            DamagePlayer(10.0f SYNC);
            break;

        case 1:
            DamagePlayer(20.0f SYNC);
            break;

        case 2:
            DamagePlayer(30.0f SYNC);
            break;

        case 3:
            DamagePlayer(40.0f SYNC);
            break;
        }


        if ((Handlung == RADELN ||
                Handlung == RADELN_FALL) &&
                WheelMode == false)
            Armour -= 5.0f SYNC;
    }


    //-----------------------------------------------
    // Testen, ob sich der Spieler im Wasser befindet

    //DKS - Added bounds check to Tiles[][] array, eliminated divisions while I was here:
    //uint32_t middle = TileEngine.TileAt((int)(xpos + 35) / TILESIZE_X, (int)(ypos + 40) / TILESIZE_Y).Block;
    uint32_t middle = 0;
    { 
        int tile_x = (xpos + 35.0f) * (1.0f/TILESIZE_X);
        int tile_y = (ypos + 40.0f) * (1.0f/TILESIZE_Y);
        if (tile_x >= 0 && tile_x < levelsize_x &&
            tile_y >= 0 && tile_y < levelsize_y)
            middle = TileEngine.TileAt(tile_x, tile_y).Block;
    }

    int spritzertype = 0;
    if ((bu & BLOCKWERT_LIQUID) ||
            (br & BLOCKWERT_LIQUID) ||
            (bl & BLOCKWERT_LIQUID) ||
            (bo & BLOCKWERT_LIQUID) ||
            middle & BLOCKWERT_LIQUID)
    {
        spritzertype = WASSER_SPRITZER2;

        // Gerade erst in Fl�ssigkeit gesprungen ?
        if (InLiquid == false)
        {
            if (Handlung == SPRINGEN ||
                    Handlung == RADELN_FALL)
            {
                for (int i=0; i<12; i++)
                    PartikelSystem.PushPartikel(xpos+18+rand()%20, ypos + CollideRect.bottom-25, spritzertype);

                WinkelUebergabe = -1.0f;
                for (int i=0; i<15; i++)
                    PartikelSystem.PushPartikel(xpos+10+rand()%40, ypos + CollideRect.bottom + rand()%20, BUBBLE);
            }

            InLiquid = true;
            SoundManager.PlayWave(100, 128, 10000 + rand()%2050, SOUND_WATERIN);
        }
    }
    else
    {
        // Aus dem Wasser heraus gehopst ?
        if (InLiquid == true)
        {
            // Welcher Liquid Type?
            //if (bu & BLOCKWERT_WASSER) spritzertype = WASSER_SPRITZER;
            //if (bu & BLOCKWERT_LAVA)   spritzertype = LAVA_SPRITZER;
            spritzertype = WASSER_SPRITZER2;

            if (Handlung == SPRINGEN ||
                    Handlung == RADELN_FALL)
            {
                for (int i=0; i<12; i++)
                    PartikelSystem.PushPartikel(xpos+10+rand()%20, ypos + CollideRect.bottom-25, spritzertype);
            }

            SoundManager.PlayWave(100, 128, 10000 + rand()%2050, SOUND_WATEROUT);
            InLiquid = false;
        }
    }

    // Spieler im Wasser
    if (InLiquid   == true)
    {
        //Blubberblasen entstehen lassen
        //DKS - No need to do two separate rand()%500 calculations, also added 
        //      function WaveIsPlaying to SoundManagerClass:
        //if (rand()%500 == 0)
        //    PartikelSystem.PushPartikel(xpos+30, ypos+20, BUBBLE);

        //// ggf noch Tauchger�usche abspielen
        //if (rand()%500 == 0 &&
        //        SoundManager.its_Sounds[SOUND_DIVE]->isPlaying == false)
        //    SoundManager.PlayWave(100, rand()%255, 8000 + rand()%4000, SOUND_DIVE);
        if (rand()%500 == 0) {
            PartikelSystem.PushPartikel(xpos+30, ypos+20, BUBBLE);
            // ggf noch Tauchger�usche abspielen
            if (!SoundManager.WaveIsPlaying(SOUND_DIVE))
                SoundManager.PlayWave(100, rand()%255, 8000 + rand()%4000, SOUND_DIVE);
        }
    }

    // schr�g laufen?
    if ((Handlung == SCHIESSEN_LO ||
            Handlung == SCHIESSEN_RO) &&
            WalkLock == false)
    {
        // N�chste Animations-Phase ?
        AnimCount += 1.0f SYNC;
        while (AnimCount > PLAYER_ANIMSPEED)
        {
            AnimCount = AnimCount - PLAYER_ANIMSPEED;

            AnimPhase++;

            if (AnimPhase >= FRAMES_RUN)
                AnimPhase = 0;	// Loop
        }
    }

    // Normal laufen oder radeln?
    if (Handlung == LAUFEN      ||					// Spieler l�uft oder
            Handlung == RADELN      ||					// oder kullert
            Handlung == RADELN_FALL ||					// oder f�llt als Rad
            (Handlung == SPRINGEN &&						// springt, sprich, wird animiert ?
             yspeed >-PLAYER_MAXJUMPSPEED/1.5f))
    {
        if (InLiquid == false)
            AnimCount += 1.0f SYNC;					// Dann animieren, je nachdem, ob man im
        else										// Wasser ist oder nicht
            AnimCount += 0.5f SYNC; 				// verschieden schnell animieren

        if (Handlung == RADELN	||
                Handlung == RADELN_FALL)				// Als Rad schneller animieren, also
            AnimCount += 4.0f SYNC;					// einfach nochmal den Wert viermal dazu

        while (AnimCount > PLAYER_ANIMSPEED)
        {

            AnimCount = AnimCount - PLAYER_ANIMSPEED;

            switch(Handlung)						// Je nach Handlung anders animieren
            {
            case LAUFEN:						// Laufen animieren
            {
                AnimPhase++;

                if (AnimPhase >= FRAMES_RUN)
                    AnimPhase = 0;	// Loop
            }
            break;

            case SPRINGEN:						// Springen animieren wobei erst animiert
            {
                // wird wenn der Spieler wieder am Fallen ist
                if (AnimPhase < FRAMES_JUMP - 1)
                    AnimPhase++;						// Kein Loop
            }
            break;

            case RADELN:
            case RADELN_FALL:
            {
                if (AnimPhase < PLAYERANIM_RAD_ENDE)
                    AnimPhase++;
                else
                    AnimPhase=0;
            }
            break;

            default :
                break;
            }
        }
    }

    // Springen
    if (Handlung == SPRINGEN ||
            (Handlung == BLITZEN &&
             yspeed > 0.0f))
    {
        // Sprung-Geschwindigkeit manipulieren
        if (InLiquid == false)
            yspeed += JumpAdd SYNC;
        else
            yspeed += JumpAdd*2/3 SYNC;

        // y-Position manipulieren, wenn oben frei ist
        if (InLiquid == false)
            ypos	  += yspeed SYNC;
        else
            ypos	  += yspeed*2/3 SYNC;

        if (yspeed > PLAYER_MAXJUMPSPEED)	// Schnellste "Fall-Geschwindigkeit" erreicht ?
            yspeed = PLAYER_MAXJUMPSPEED;

        if (InLiquid == false)
        {
            if (ypos < JumpStart - PLAYER_MAXJUMP ||	// H�chster Punkt erreicht ? Dann den Fall
                    Aktion[AKTION_JUMP] == false)			// dazu addieren
            {
                // Der Wert, wie schnell man wieder runterkommt, wird jetzt aus der Sprungh�he errechnet, sodass
                // man bei einem kleinen Sprung viel schneller wieder runterkommt
                //
                JumpAdd = PLAYER_JUMPADDSPEED;
                JumpAdd = PLAYER_JUMPADDSPEED + abs (int ((160 - (JumpySave - ypos)) / 10.0f));

                if (JumpAdd > 18.0f)
                    JumpAdd = 18.0f;
            }
        }
        else
        {
            if (ypos < JumpStart - (PLAYER_MAXJUMP+50) ||	// H�chster Punkt im Wasser liegt anders
                    Aktion[AKTION_JUMP] == false)				// dazu addieren
                JumpAdd = PLAYER_JUMPADDSPEED;
        }
    }

    // Als Rad runterfallen
    if (Handlung == RADELN_FALL)
    {
        // Sprung-Geschwindigkeit manipulieren
        if (InLiquid == false)
            yspeed += JumpAdd SYNC;
        else
            yspeed += JumpAdd*2/3 SYNC;

        // y-Position manipulieren, wenn oben frei ist
        if (InLiquid == false)
            ypos	  += yspeed SYNC;
        else
            ypos	  += yspeed*2/3 SYNC;

        if (yspeed > PLAYER_MAXJUMPSPEED)	// Schnellste "Fall-Geschwindigkeit" erreicht ?
            yspeed = PLAYER_MAXJUMPSPEED;

        if(bo & BLOCKWERT_WAND)	// An die Decke gestossen ?
        {
            if (yspeed < 0.0f)					// Richtung umkehren
                yspeed = -yspeed/3;
            JumpAdd = PLAYER_JUMPADDSPEED;
        }
    }

    // Spieler "rutscht" weg, weil Level schr�g steht?
    if (bu & BLOCKWERT_WAND ||
            bu & BLOCKWERT_PLATTFORM)
        xspeed += ScreenWinkel * 7.0f SYNC;

    // Level abh�ngig von der Spieler-Position scrollen
    if (xspeed != 0.0f)
    {
        // nur bewegen, wenn keine Wand im Weg ist
        if ((xspeed < 0.0f && !(bl & BLOCKWERT_WAND)) ||
                (xspeed > 0.0f && !(br & BLOCKWERT_WAND)))
            xpos += xspeed SYNC;								 // Spieler bewegen*/

        // Zwei Spieler Mode? Dann auf Screen beschr�nken
        if (NUMPLAYERS == 2 &&
                StageClearRunning == false)
        {
            if (xpos < TileEngine.XOffset)  xpos = (float)TileEngine.XOffset;
            if (xpos > TileEngine.XOffset + 570) xpos = (float)TileEngine.XOffset + 570;
        }
    }

    /*if (Handlung != SPRINGEN	&&
    	Handlung != RADELN_FALL &&
    	yspeed <= 0.0f)
    	TileEngine.YOffset -= PLAYER_MAXJUMPSPEED SYNC;*/


    if (NUMPLAYERS == 1 &&
            TileEngine.Zustand == ZUSTAND_SCROLLBAR)
    {
        if(xpos-TileEngine.XOffset <  20) TileEngine.XOffset = xpos - 20;
        if(xpos-TileEngine.XOffset > 550)	TileEngine.XOffset = xpos - 550;
    }

    //DKS - By using a bounds-checked array for itsPreCalcedRects[] when debugging,
    //      I found AnimPhase was going out of bounds of the number of frames when
    //      Handlung==LAUFEN (running), at times. So, I've added some checks here
    //      to ensure that never happens:
    switch(Handlung)
    {
        case RADELN:
        case RADELN_FALL:
            if (AnimPhase >= 8)
                AnimPhase = 0;
            break;
        case LAUFEN:
            //DKS - This was the problematic one causing out-of-bounds access to itsPreCalcedRects
            if (AnimPhase >= FRAMES_RUN)
                AnimPhase = 0;
            break;
        case BLITZEN :
        case BEAMLADEN:
            if (AnimPhase >= 36)
                AnimPhase = 0;
            break;
        case DREHEN :
            if (AnimPhase >= 20)
                AnimPhase = 0;
            break;
        case SPRINGEN:
            if (AnimPhase >= 12)
                AnimPhase = 0;
            break;
        case PISSEN:
            if (AnimPhase >= 18)
                AnimPhase = 0;
            break;
        default:
            break;
    }
}

// --------------------------------------------------------------------------------------
// Aktionen, wenn der Spieler auf einer Plattform steht
// --------------------------------------------------------------------------------------

void PlayerClass::DoPlattformStuff(void)
{
    if (AufPlattform == NULL)
        return;

    if (AufPlattform->GegnerArt == REITFLUGSACK)
        return;

    if (Handlung == SPRINGEN)
    {
        Handlung = STEHEN;
        yspeed   = 0.0f;
    }

    if (Aktion[AKTION_JUMP] == false)
        JumpPossible = true;

    // Fahrstuhl �ber Boden gefahren? Dann bleibt der Spieler daran h�ngen
    if (bu & BLOCKWERT_WAND ||
            bu & BLOCKWERT_PLATTFORM)
    {
        AufPlattform = NULL;
    }
    else
    {
        if (AufPlattform->GegnerArt == FAHRSTUHL)
            ypos = g_Fahrstuhl_yPos + GegnerRect[AufPlattform->GegnerArt].top - CollideRect.bottom;
        else
            ypos = AufPlattform->yPos + GegnerRect[AufPlattform->GegnerArt].top - CollideRect.bottom;
    }
}

// --------------------------------------------------------------------------------------
// Spieler an der aktuellen Position zeichnen, je nach Zustand
//
//	leuchten : Spieler additiv rendern
//  farbe    : mit Farbe "CurrentColor" rendern
// --------------------------------------------------------------------------------------

bool PlayerClass::DrawPlayer(bool leuchten, bool farbe)
{
    if (Energy <= 0.0f &&
            Handlung == TOT)
        return false;

    float		xdraw, ydraw;
    D3DCOLOR	Color;

    if (leuchten)
        DirectGraphics.SetAdditiveMode();

    xdraw = (float)((int)(xpos) - (int)(TileEngine.XOffset));
    ydraw = ypos - TileEngine.YOffset;

    // Im Wasser? Dann schwabbeln lassen
    //DKS - This original block of code never had an effect because there are no platforms
    //      underwater that I know of. Also, when I had it also execute when standing on
    //      a BLOCKWERT_WAND tile, I discovered the left/right swaying was very ugly
    //      and not even in coordination with other artwork lying in water. I've disabled
    //      it entirely:
#if 0
    if (InLiquid == true &&
            bo & BLOCKWERT_LIQUID &&
            bu & BLOCKWERT_PLATTFORM)
    {
        int yLevel = (int)(ypos + 80.0f) / 20;
        int off = (int)(TileEngine.SinPos2 + yLevel) % 40;

        xdraw -= TileEngine.SinList2[off];
    }
#endif //0

    // Schaden genommen ? Dann Spieler blinken lassen
    if (DamageCounter > 0.0f)
        DamageCounter -= 5.0f SYNC;
    else
        DamageCounter = 0.0f;

    //DKS - No need to check if it is playing, just ask it to stop:
    //if (DamageCounter == 0.0f &&
    //        true == SoundManager.InitSuccessfull &&
    //        SoundManager.its_Sounds[SOUND_ABZUG + SoundOff]->isPlaying == true)
    //    SoundManager.StopWave(SOUND_ABZUG + SoundOff);
    if (DamageCounter == 0.0f)
        SoundManager.StopWave(SOUND_ABZUG + SoundOff);

    Color = TileEngine.LightValue(xpos, ypos, CollideRect, false);

    if (Handlung == BLITZEN)
        Color = Color | 0xFF0000FF;

    if (ShotDelay > 1.0f)
        Color = 0xFFFFFFFF;

    // Spieler nicht normal rendern (zB wenn er blinkt nach dem Waffeneinsammeln)
    // dann entsprechende Farbe setzen
    if (farbe)
        Color = CurrentColor;

//	if (this == Player[1])
//		Color = 0xFFBBFFBB;

    bool blick = false;
    if (Blickrichtung == RECHTS)
        blick = true;

//----- Spieler anzeigen
    switch (Handlung)
    {

// Spieler kullert (oder f�llt) als Rad durch die Gegend
    case RADELN:
    case RADELN_FALL:
    {
        PlayerRad.RenderSprite(xdraw+17, ydraw+45, AnimPhase, Color, blick);
    }
    break;

// Spieler steht und macht nichts
    case STEHEN :
    {
        if (Aktion[AKTION_UNTEN] &&
                !Aktion[AKTION_SHOOT] &&
                !Aktion[AKTION_GRANATE])
            PlayerKucken.RenderSprite(xdraw, ydraw, 1, Color, !blick);
        else
        {
            int a;

            if (AnimPhase < FRAMES_IDLE * 3)
            {
                a = AnimPhase % FRAMES_IDLE;

                if (ShotDelay > 0.25f &&
                        FlameThrower == false)
                    PlayerIdle.RenderSprite(xdraw, ydraw, FRAMES_IDLE, Color, !blick);
                else
                    PlayerIdle.RenderSprite(xdraw, ydraw, a, Color, !blick);
            }
            else
            {
                a = AnimPhase - FRAMES_IDLE * 3;
                PlayerIdle2.RenderSprite(xdraw, ydraw, a, Color, !blick);
            }
        }

    }
    break;

// Spieler zielt nach oben
    case SCHIESSEN_O :
    {
        if (Aktion[AKTION_SHOOT] ||
                Aktion[AKTION_GRANATE])
        {
            if (ShotDelay > 0.25f &&
                    FlameThrower == false)
                PlayerOben.RenderSprite(xdraw, ydraw, 1, Color, !blick);
            else
                PlayerOben.RenderSprite(xdraw, ydraw, 0, Color, !blick);
        }
        else
            PlayerKucken.RenderSprite(xdraw, ydraw, 0, Color, !blick);
    }
    break;

// Spieler zielt nach links oder rechts oben
    case SCHIESSEN_LO:
    case SCHIESSEN_RO:
    {
        // im Stehen schiessen?
        if (WalkLock == true)
        {
            if (ShotDelay > 0.25f &&
                    FlameThrower == false)
                PlayerDiagonal.RenderSprite(xdraw, ydraw, 1, Color, !blick);
            else
                PlayerDiagonal.RenderSprite(xdraw, ydraw, 0, Color, !blick);
        }

        /*
        			// oder im Laufen?
        			else
        			{
        				if (ShotDelay > 0.25f)
        					PlayerRunDiagonal[AnimPhase].RenderSprite(xdraw, ydraw, 0, Color, !blick);
        				else
        					PlayerRunDiagonal[AnimPhase].RenderSprite(xdraw, ydraw, 0, Color, !blick);
        			}
        */
    }
    break;

// Spieler zielt nach links oder rechts unten
    /*		case SCHIESSEN_LU:
    		case SCHIESSEN_RU:
    		{
    			PlayerBlitz.RenderSprite(xdraw, ydraw, 4, Color, blick);
    		} break;*/

    //DKS - This "surfing" appears never to have been implemented (no image for it) so disabled it:
#if 0
// Spieler surft
    case SURFEN :
    {
        PlayerSurf.RenderSprite(xdraw, ydraw, 0, Color, false);
    }
    break;

// Spieler springt bei surfen
    case SURFENJUMP :
    {
        PlayerSurf.RenderSprite(xdraw, ydraw, 0, Color, false);
    }
    break;

// Spieler surft im Kien
    case SURFENCROUCH :
    {
        PlayerSurf.RenderSprite(xdraw, ydraw, 1, Color, false);
    }
    break;
#endif //0


// Spieler springt
    case SPRINGEN :
    {
        if (Aktion[AKTION_OBEN])
        {
            if (Aktion[AKTION_LINKS] ||
                    Aktion[AKTION_RECHTS])
                PlayerJumpDiagonal.RenderSprite(xdraw, ydraw, AnimPhase, Color, !blick);
            else
                PlayerJumpUp.RenderSprite(xdraw, ydraw, AnimPhase, Color, !blick);
        }
        else
            PlayerJump.RenderSprite(xdraw, ydraw, AnimPhase, Color, !blick);

    }
    break;

// Spieler l�uft
    case LAUFEN :
    {
        PlayerRun.RenderSprite(xdraw, ydraw, AnimPhase, Color, !blick);
    }
    break;

// Spieler muss brutal pullern ?
    case PISSEN:
    {
        PlayerPiss.RenderSprite(xdraw, ydraw, AnimPhase, Color, !blick);
    }
    break;

// Spieler schiesst Blitz oder l�dt den Beam
    case BLITZEN :
    case BEAMLADEN:
    {
        PlayerBlitz.RenderSprite(xdraw, ydraw, AnimPhase, Color, !blick);
    }
    break;

// Spieler duckt sich
    case DUCKEN :
    {
        if (ShotDelay > 0.25f)
            PlayerCrouch.RenderSprite(xdraw, ydraw, 1, Color, !blick);
        else
            PlayerCrouch.RenderSprite(xdraw, ydraw, 0, Color, !blick);
    }
    break;

// Spieler reitet auf dem FlugSack
    case SACKREITEN :
    {
        if (Blickrichtung == LINKS)
            PlayerRide.RenderSprite(xdraw, ydraw, 0, Color);
        else if (Blickrichtung == RECHTS)
            PlayerRide.RenderSprite(xdraw, ydraw, 10, Color);
    }
    break;

    case DREHEN :
    {
        PlayerRide.RenderSprite(xdraw, ydraw,    AnimPhase,    Color);
    }
    break;

    default :
        break;
    }

    //----- Schuss-Flamme anzeigen
    if (FlameTime > 0.0f  &&
            leuchten == false &&
            farbe    == false)

    {
        // Zeit der Darstellung verringern
        //
        FlameTime -= SpeedFaktor;

        DirectGraphics.SetAdditiveMode();
        CalcFlamePos();

        int FlameOff = SelectedWeapon;

        if (FlameThrower)
            FlameOff = 1;

        //DKS - Added check that AustrittAnim was between 0-2, because in the original code,
        //      SchussFlamme[] had 4 elements, but only the first 3 elements actually were
        //      ever loaded with textures.
        if (AustrittAnim >= 0 && AustrittAnim < 3) {
            if (Blickrichtung == RECHTS)
                Projectiles.SchussFlamme[AustrittAnim].RenderSprite(xpos + AustrittX, ypos + AustrittY, FlameAnim+FlameOff*2, 0xFFFFFFFF);
            else
                Projectiles.SchussFlamme[AustrittAnim].RenderSprite(xpos + AustrittX, ypos + AustrittY, FlameAnim+FlameOff*2, 0xFFFFFFFF, true);
        }

        CalcAustrittsPunkt();

        if (options_Detail >= DETAIL_MEDIUM)
        {
            int FlameOff = SelectedWeapon;
            if (FlameThrower)
                FlameOff = 0;

            switch (FlameOff)
            {
            case 0 :
                Projectiles.SchussFlammeFlare.RenderSprite(xpos + AustrittX - 70 - (float)TileEngine.XOffset,
                                               ypos + AustrittY - 70 - (float)TileEngine.YOffset, 0, 0x88FFCC99);
                break;

            case 1 :
                Projectiles.SchussFlammeFlare.RenderSprite(xpos + AustrittX - 70 - (float)TileEngine.XOffset,
                                               ypos + AustrittY - 70 - (float)TileEngine.YOffset, 0, 0x8899CCFF);
                break;

            case 2 :
                Projectiles.SchussFlammeFlare.RenderSprite(xpos + AustrittX - 70 - (float)TileEngine.XOffset,
                                               ypos + AustrittY - 70 - (float)TileEngine.YOffset, 0, 0x8899FFCC);
                break;
            }
        }

        DirectGraphics.SetColorKeyMode();
    }

    // Blitz anzeigen oder Beam aufladen
    //
    if (Handlung == BLITZEN &&
            AlreadyDrawn == false)
        DoLightning();

    if (Handlung == BEAMLADEN &&
            AlreadyDrawn == false)
        LoadBeam();

    if (leuchten)
        DirectGraphics.SetColorKeyMode();

    // Funken und so Kram, wenn der Spieler verwundet ist
    //
    if (Console.Showing == false)
        DoStuffWhenDamaged();

    CurrentShotTexture = 0;
    CurrentPartikelTexture = -1;

    AlreadyDrawn = true;

    return true;
}

// --------------------------------------------------------------------------------------
// Spieler mit aktueller Speed bewegen
// --------------------------------------------------------------------------------------

void PlayerClass::MovePlayer(void)
{
    // wenn der Spieler gerade fernegesteuert das Level verl�sst dann gleich wieder raus
    if (DoFesteAktion == true ||
            Handlung == TOT)
        return;

    // R�nder �berpr�fen
    // links raus
    if (xpos < 0.0f)
        xpos = 0.0f;

    // rechts raus
    if (xpos > TileEngine.LEVELPIXELSIZE_X - 80.0f)
        xpos = float(TileEngine.LEVELPIXELSIZE_X - 80.0f);

    // unten raus
    if (ypos > TileEngine.LEVELPIXELSIZE_Y)
        Energy = 0.0f;

    // im 2 Spieler-Mode: Ein Spieler f�llt aus dem Screen?
    // Geht nur, wenn beide noch leben
    //
    if (NUMPLAYERS == 2 &&
            Player[0].Handlung != TOT &&
            Player[1].Handlung != TOT)
    {
        PlayerClass *pVictim = NULL;
        PlayerClass *pSurvivor = NULL;

        // Spieler 1 f�llt unten raus?
        if (Player[0].ypos > Player[1].ypos + 480.0f)
        {
            pVictim   = &Player[0];
            pSurvivor = &Player[1];
        }

        // Spieler 2 f�llt unten raus?
        if (Player[1].ypos > Player[0].ypos + 480.0f)
        {
            pVictim   = &Player[1];
            pSurvivor = &Player[0];
        }

        // Einer der Spieler f�llt raus
        if (pVictim != NULL)
        {
            pVictim->xpos = pSurvivor->xpos;
            pVictim->ypos = pSurvivor->ypos - 20.0f;
        }
    }



    // an die Decke
    if (ypos < 0.0f)
    {
        if (yspeed < 0.0f)
            yspeed = -yspeed * 2/3;
        else
            ypos = 0.0f;
    }

    // R�nder f�r gelockten Screen pr�fen
    if (TileEngine.Zustand != ZUSTAND_SCROLLBAR)
    {
        if (xpos < TileEngine.XOffset)	 xpos = float(TileEngine.XOffset);
        if (xpos > TileEngine.XOffset+580) xpos = float(TileEngine.XOffset+580);

        // Im Fahrstuhllevel?
        //
        if (g_Fahrstuhl_yPos > -1.0f)
        {
            // ja, dann checken ob Spieler ausserhalb des Screens, und wenn ja, dann
            // lassen wir ihn halt mal einfach so sterben. Das geh�rt sich ja auch nicht ;)
            //
            if (ypos + CollideRect.bottom < TileEngine.YOffset ||
                    ypos > TileEngine.YOffset + 480.0f)
                Energy = 0.0f;
        }
        else
        {
            if (ypos < TileEngine.YOffset)
                ypos = float(TileEngine.YOffset);
        }

    }
}

// --------------------------------------------------------------------------------------
// Spieler schiesst mit Prim�r Waffe
// --------------------------------------------------------------------------------------

void PlayerClass::PlayerShoot(void)
{
    float wadd = 0.0f;
    int   yoff;

    CalcAustrittsPunkt();

    //----- Y-Offset der Patronenh�lse ausrechnen

    //DKS - This appears never to have been implemented (no image for it) so disabled it:
#if 0
    // surfen
    if (Handlung == SURFEN ||
            Handlung == SURFENJUMP)
        yoff = -13;
    else if (Handlung == SURFENCROUCH)
        yoff = -1;

    // andere Handlung
    else
#endif //0
    {
        if (Handlung == DUCKEN)
            yoff = 23;
        else
            yoff = 0;
    }

    //----- Richung des Schusses ermitteln

    // Hurri zielt nach oben
    //
    if (Handlung == SCHIESSEN_O ||
            (Handlung == SPRINGEN &&
             Aktion[AKTION_OBEN]  &&
             !Aktion[AKTION_LINKS] &&
             !Aktion[AKTION_RECHTS]))
    {
        if (Blickrichtung == LINKS)
            wadd = 90.0f;
        else
            wadd = -90.0f;
    }
    else

        // Hurri zielt nach links oben
        //
        if (Handlung == SCHIESSEN_LO ||
                (Handlung == SPRINGEN &&
                 Aktion[AKTION_OBEN]  &&
                 Aktion[AKTION_LINKS]))
            wadd = 45.0f;
        else

            // Hurri zielt nach rechts oben
            //
            if (Handlung == SCHIESSEN_RO ||
                    (Handlung == SPRINGEN &&
                     Aktion[AKTION_OBEN]  &&
                     Aktion[AKTION_RECHTS]))
                wadd = -45.0f;

    if (Handlung != RADELN &&						// Normal schiessen ?
            Handlung != RADELN_FALL)
    {

        int tempadd = 0;
        int tempshot = 0;

        if (FlameThrower)
        {
            WinkelUebergabe = 90.0f + wadd;
            Projectiles.PushProjectile(xpos-tempadd+AustrittX - 29, ypos-tempadd+AustrittY - 33, PLAYERFIRE, this);
            ShotDelay = PLAYER_SHOTDELAY / 5.0f;

            //DKS - Added function WaveIsPlaying() to SoundManagerClass:
            if (!SoundManager.WaveIsPlaying(SOUND_FLAMETHROWER + SoundOff))
                SoundManager.PlayWave(100, 128, 11025, SOUND_FLAMETHROWER + SoundOff);
        }
        else
            switch (SelectedWeapon)
            {

//----- Spread Shot

            case 0 :
            {
                // Normale Sch�sse
                //
                if (RiesenShotExtra <= 0.0f)
                {
                    tempadd = 8;
                    tempshot = SPREADSHOT;
                    SoundManager.PlayWave(100, 128, 11025, SOUND_SPREADSHOT);
                }
                else
                {
                    tempadd = 16;
                    tempshot = SPREADSHOTBIG;
                    SoundManager.PlayWave(100, 128, 8000, SOUND_SPREADSHOT);
                }

                WinkelUebergabe = 90.0f + wadd;

                if (CurrentWeaponLevel[SelectedWeapon] < 5)
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot, this);

                if (CurrentWeaponLevel[SelectedWeapon] >= 5)
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);

                if (CurrentWeaponLevel[SelectedWeapon] > 1 &&
                        CurrentWeaponLevel[SelectedWeapon] < 6)
                {
                    WinkelUebergabe = 82.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot, this);

                    WinkelUebergabe = 98.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot, this);
                }

                if (CurrentWeaponLevel[SelectedWeapon] >= 6)
                {
                    WinkelUebergabe = 82.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);

                    WinkelUebergabe = 98.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);
                }

                if (CurrentWeaponLevel[SelectedWeapon] > 2 &&
                        CurrentWeaponLevel[SelectedWeapon] < 7)
                {
                    WinkelUebergabe = 74.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot, this);

                    WinkelUebergabe = 106.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot, this);
                }

                if (CurrentWeaponLevel[SelectedWeapon] >= 7)
                {
                    WinkelUebergabe = 74.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);

                    WinkelUebergabe = 106.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);
                }

                if (CurrentWeaponLevel[SelectedWeapon] > 3 &&
                        CurrentWeaponLevel[SelectedWeapon] < 8)
                {
                    WinkelUebergabe = 66.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot, this);

                    WinkelUebergabe = 114.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot, this);
                }

                if (CurrentWeaponLevel[SelectedWeapon] >= 8)
                {
                    WinkelUebergabe = 66.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);

                    WinkelUebergabe = 114.0f + wadd;
                    Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);
                }
            }
            break;

//----- LaserShot

            case 1 :
            {
                int tempaddx, tempaddy, tempshot;
                float mul1 = 7.0f/8.0f;

                // Normale Sch�sse
                //
                if (RiesenShotExtra <= 0.0f)
                {
                    tempaddx = 8;
                    tempaddy = 24;
                    tempshot = LASERSHOT;
                    SoundManager.PlayWave(100, 128, 11025, SOUND_LASERSHOT);
                }
                else
                {
                    tempaddx = 12;
                    tempaddy = 40;
                    tempshot = LASERSHOTBIG;
                    SoundManager.PlayWave(100, 128, 8000, SOUND_LASERSHOT);
                }

                WinkelUebergabe = 90.0f + wadd;

                if (CurrentWeaponLevel[SelectedWeapon] == 4 ||
                        CurrentWeaponLevel[SelectedWeapon] == 6)
                    Projectiles.PushProjectile(xpos-tempaddx+AustrittX - 4, ypos-tempaddy+AustrittY, tempshot + 1, this);

                if (CurrentWeaponLevel[SelectedWeapon] == 1 ||
                        CurrentWeaponLevel[SelectedWeapon] == 3 ||
                        CurrentWeaponLevel[SelectedWeapon] == 5)
                    Projectiles.PushProjectile(xpos-tempaddx+AustrittX, ypos-tempaddy+AustrittY, tempshot, this);

                if (CurrentWeaponLevel[SelectedWeapon] == 2)
                {
                    if (wadd == 0.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX, ypos-tempaddy+AustrittY-tempaddx*mul1, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX, ypos-tempaddy+AustrittY+tempaddx*mul1, tempshot, this);
                    }

                    if (wadd == 45.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+tempaddx*mul1+AustrittX - 2, ypos-tempaddy+AustrittY-tempaddx*mul1, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx-tempaddx*mul1+AustrittX + 2, ypos-tempaddy+AustrittY+tempaddx*mul1, tempshot, this);
                    }

                    if (wadd == -45.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx-tempaddx*mul1+AustrittX+6, ypos-tempaddy+AustrittY-tempaddx*mul1, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx+tempaddx*mul1+AustrittX+2, ypos-tempaddy+AustrittY+tempaddx*mul1, tempshot, this);
                    }

                    if (wadd ==  90.0f ||
                            wadd == -90.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*mul1, ypos-tempaddy+AustrittY, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX+tempaddx*mul1, ypos-tempaddy+AustrittY, tempshot, this);
                    }
                }

                if (CurrentWeaponLevel[SelectedWeapon] >= 7)
                {
                    if (wadd == 0.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX, ypos-tempaddy+AustrittY-tempaddx*mul1, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX, ypos-tempaddy+AustrittY+tempaddx*mul1, tempshot + 1, this);
                    }

                    if (wadd == 45.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+tempaddx*mul1*0.75f+AustrittX, ypos-tempaddy+AustrittY-tempaddx*mul1*0.75f, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx-tempaddx*mul1*0.75f+AustrittX, ypos-tempaddy+AustrittY+tempaddx*mul1*0.75f, tempshot + 1, this);
                    }

                    if (wadd == -45.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx-tempaddx*mul1*0.75f+AustrittX, ypos-tempaddy+AustrittY-tempaddx*mul1*0.75f, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx+tempaddx*mul1*0.75f+AustrittX, ypos-tempaddy+AustrittY+tempaddx*mul1*0.75f, tempshot + 1, this);
                    }

                    if (wadd ==  90.0f ||
                            wadd == -90.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*mul1, ypos-tempaddy+AustrittY, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX+tempaddx*mul1, ypos-tempaddy+AustrittY, tempshot + 1, this);
                    }
                }

                if (CurrentWeaponLevel[SelectedWeapon] >= 3 &&
                        CurrentWeaponLevel[SelectedWeapon] <= 4)
                {
                    if (wadd == 0.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*Blickrichtung, ypos-tempaddy-tempaddx*mul1*2.3f+AustrittY, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*Blickrichtung, ypos-tempaddy+tempaddx*mul1*2.3f+AustrittY, tempshot, this);
                    }

                    if (wadd == 90.0f ||
                            wadd ==-90.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx-tempaddx*mul1*2.3f+AustrittX, ypos-tempaddy/2+AustrittY, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx+tempaddx*mul1*2.3f+AustrittX, ypos-tempaddy/2+AustrittY, tempshot, this);
                    }

                    if (wadd == 45.0f)
                    {
                        Projectiles.PushProjectile(xpos+tempaddx*mul1*2.3f+AustrittX, ypos-tempaddy+AustrittY, tempshot, this);
                        Projectiles.PushProjectile(xpos+tempaddx-tempaddx*mul1*2.3f+AustrittX, ypos-tempaddy+tempaddx*3+AustrittY, tempshot, this);
                    }

                    if (wadd == -45.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx*mul1*4+AustrittX, ypos-tempaddy+AustrittY, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx*2+tempaddx*mul1*2.3f+AustrittX, ypos-tempaddy+tempaddx*2+AustrittY, tempshot, this);
                    }
                }

                if (CurrentWeaponLevel[SelectedWeapon] == 5 ||
                        CurrentWeaponLevel[SelectedWeapon] == 6)
                {
                    if (wadd == 0.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*Blickrichtung*2, ypos-tempaddy+AustrittY-tempaddx*mul1*2, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*Blickrichtung*2, ypos-tempaddy+AustrittY+tempaddx*mul1*2, tempshot + 1, this);
                    }

                    if (wadd == 90.0f ||
                            wadd ==-90.0f)
                    {
                        Projectiles.PushProjectile(xpos+tempaddx*mul1*2-tempaddx+AustrittX - 4, ypos-tempaddy+AustrittY+tempaddx, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx*mul1*2-tempaddx+AustrittX - 4, ypos-tempaddy+AustrittY+tempaddx, tempshot + 1, this);
                    }

                    if (wadd == 45.0f)
                    {
                        Projectiles.PushProjectile(xpos+tempaddx+AustrittX-4, ypos-tempaddy+AustrittY, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-4, ypos-tempaddy+tempaddx*2+AustrittY, tempshot + 1, this);
                    }

                    if (wadd == -45.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx*3+AustrittX-6, ypos-tempaddy+AustrittY, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-4, ypos-tempaddy+tempaddx*2+AustrittY, tempshot + 1, this);
                    }
                }


                if (CurrentWeaponLevel[SelectedWeapon] == 7)
                {
                    if (wadd == 0.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*Blickrichtung*2, ypos-tempaddy+AustrittY-tempaddx*mul1*3, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*Blickrichtung*2, ypos-tempaddy+AustrittY+tempaddx*mul1*3, tempshot, this);
                    }

                    if (wadd == 45.0f)
                    {
                        Projectiles.PushProjectile(xpos+tempaddx*2+AustrittX+6, ypos-tempaddy+AustrittY, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx*2+AustrittX+4, ypos-tempaddy+tempaddx*5/2+AustrittY, tempshot, this);
                    }

                    if (wadd == -45.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx*4+AustrittX+2, ypos-tempaddy+AustrittY, tempshot, this);
                        Projectiles.PushProjectile(xpos+AustrittX+4, ypos-tempaddy+tempaddx*5/2+AustrittY, tempshot, this);
                    }

                    if (wadd ==  90.0f ||
                            wadd == -90.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx-tempaddy*mul1+AustrittX+3, ypos-tempaddy/2+AustrittY, tempshot, this);
                        Projectiles.PushProjectile(xpos-tempaddx+tempaddy*mul1+AustrittX+4, ypos-tempaddy/2+AustrittY, tempshot, this);
                    }
                }

                if (CurrentWeaponLevel[SelectedWeapon] == 8)
                {
                    if (wadd == 0.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*Blickrichtung*2, ypos-tempaddy+AustrittY-tempaddx*3, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx+AustrittX-tempaddx*Blickrichtung*2, ypos-tempaddy+AustrittY+tempaddx*3, tempshot + 1, this);
                    }

                    if (wadd == 45.0f)
                    {
                        Projectiles.PushProjectile(xpos+tempaddx*3+AustrittX, ypos-tempaddy+AustrittY, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx*2+AustrittX, ypos-tempaddy+tempaddx*3+AustrittY, tempshot + 1, this);
                    }

                    if (wadd == -45.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx*4.5f+AustrittX, ypos-tempaddy+AustrittY, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos+AustrittX, ypos-tempaddy+tempaddx*3+AustrittY, tempshot + 1, this);
                    }

                    if (wadd ==  90.0f ||
                            wadd == -90.0f)
                    {
                        Projectiles.PushProjectile(xpos-tempaddx-tempaddy*mul1+AustrittX, ypos-tempaddy/2+AustrittY, tempshot + 1, this);
                        Projectiles.PushProjectile(xpos-tempaddx+tempaddy*mul1+AustrittX, ypos-tempaddy/2+AustrittY, tempshot + 1, this);
                    }
                }

            }
            break;

//----- BounceShot

            case 2 :
            {
                int tempadd, tempshot;

                // Normale Sch�sse
                //
                if (RiesenShotExtra <= 0.0f)
                {
                    tempadd = 12;
                    tempshot = BOUNCESHOT1;

                    SoundManager.PlayWave(100, 128, 11025, SOUND_BOUNCESHOT);
                }
                else
                {
                    tempadd = 24;
                    tempshot = BOUNCESHOTBIG1;
                    SoundManager.PlayWave(100, 128, 8000, SOUND_BOUNCESHOT);
                }

                WinkelUebergabe = 90.0f + wadd;

                //if (CurrentWeaponLevel[SelectedWeapon] <= 5)
                Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot, this);

                /*
                if (CurrentWeaponLevel[SelectedWeapon] == 5)
                {
                	if (wadd == 0.0f)
                	{
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX + 4, ypos-tempadd+AustrittY-11, tempshot+1, this);
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX + 4, ypos-tempadd+AustrittY+19, tempshot+1, this);
                	}
                	else
                	if (wadd ==  90.0f ||
                		wadd == -90.0f)
                	{
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX-11, ypos-tempadd+AustrittY+4, tempshot+1, this);
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX+19, ypos-tempadd+AustrittY+4, tempshot+1, this);
                	}
                	else
                	if (wadd == 45.0f)
                	{
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX-7, ypos-tempadd+AustrittY+17, tempshot+1, this);
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX+17, ypos-tempadd+AustrittY-7, tempshot+1, this);
                	}
                	else
                	if (wadd == -45.0f)
                	{
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX-7, ypos-tempadd+AustrittY-7, tempshot+1, this);
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX+17, ypos-tempadd+AustrittY+15, tempshot+1, this);
                	}
                }

                if (CurrentWeaponLevel[SelectedWeapon] > 5)
                {
                	if (wadd ==  0.0f)
                	{
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY-10, tempshot, this);
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY+10, tempshot, this);
                	}
                	else
                	if (wadd ==  90.0f ||
                		wadd == -90.0f)
                	{
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX-10, ypos-tempadd+AustrittY, tempshot, this);
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX+10, ypos-tempadd+AustrittY, tempshot, this);
                	}

                	if (wadd == -45.0f)
                	{
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX-7, ypos-tempadd+AustrittY-7, tempshot, this);
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX+7, ypos-tempadd+AustrittY+7, tempshot, this);
                	}
                	else
                	if (wadd == 45.0f)
                	{
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX+7, ypos-tempadd+AustrittY-7, tempshot, this);
                		Projectiles.PushProjectile(xpos-tempadd+AustrittX-7, ypos-tempadd+AustrittY+7, tempshot, this);
                	}
                }

                if (CurrentWeaponLevel[SelectedWeapon] == 7)
                {
                	WinkelUebergabe = 60.0f + wadd;
                	Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+2, this);

                	WinkelUebergabe = 120.0f + wadd;
                	Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+2, this);
                }

                if (CurrentWeaponLevel[SelectedWeapon] == 8)
                {
                	WinkelUebergabe = 60.0f + wadd;
                	Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);

                	WinkelUebergabe = 120.0f + wadd;
                	Projectiles.PushProjectile(xpos-tempadd+AustrittX, ypos-tempadd+AustrittY, tempshot+1, this);
                }
                */

            }
            break;

            default :
                break;
            } // switch

        if (!FlameThrower)
        {
            // Patronenh�lse auswerfen bei SpreadShot
            if (SelectedWeapon == 0)
                PartikelSystem.PushPartikel(xpos+30, ypos+24+yoff, BULLET, this);
            else

                // Rauchwolke bei Laser
                if (SelectedWeapon == 1)
                    PartikelSystem.PushPartikel(xpos+30, ypos+24+yoff, SMOKE3);
                else

                    // Gr�ne Funken beim Bounce
                    if (SelectedWeapon == 2)
                        for (int i = 0; i < 2; i++)
                            PartikelSystem.PushPartikel(xpos+30 + rand ()%4, ypos+28+yoff + rand ()%4, FUNKE2);
        }

        // Schussflamme
        FlameTime = 1.0f;
        FlameAnim = rand()%2;
    }

    else					// oder Bombe legen
    {
        // Riesen Bombe?
        if (RiesenShotExtra > 0.0f)
            Projectiles.PushProjectile(xpos+26, ypos+46, BOMBEBIG, this);

        // oder normal
        else
            Projectiles.PushProjectile(xpos+30, ypos+56, BOMBE, this);
    }
}

// --------------------------------------------------------------------------------------
// Spieler Granate
// --------------------------------------------------------------------------------------

void PlayerClass::PlayerGrenadeShoot (void)
{
    CalcAustrittsPunkt();

    //----- Richung des Schusses ermitteln

    float wadd = 0.0f;

    // Hurri zielt nach oben
    //
    if (Handlung == SCHIESSEN_O ||
            (Handlung == SPRINGEN &&
             Aktion[AKTION_OBEN]  &&
             !Aktion[AKTION_LINKS] &&
             !Aktion[AKTION_RECHTS]))
    {
        if (Blickrichtung == LINKS)
            wadd = 90.0f;
        else
            wadd = -90.0f;
    }
    else

        // Hurri zielt nach links oben
        //
        if (Handlung == SCHIESSEN_LO ||
                (Handlung == SPRINGEN &&
                 Aktion[AKTION_OBEN]  &&
                 Aktion[AKTION_LINKS]))
            wadd = 45.0f;
        else

            // Hurri zielt nach rechts oben
            //
            if (Handlung == SCHIESSEN_RO ||
                    (Handlung == SPRINGEN &&
                     Aktion[AKTION_OBEN]  &&
                     Aktion[AKTION_RECHTS]))
                wadd = -45.0f;

    WinkelUebergabe = 90.0f + wadd;
    Projectiles.PushProjectile(xpos+AustrittX-4, ypos+AustrittY-4, GRENADE, this);

    SoundManager.PlayWave(100, 128, 11025, SOUND_GRANATE);
    PowerLinePossible = false;			// Taste einrasten
    Grenades--;							// Granaten verringern

    // Schussflamme
    FlameTime = 1.0f;
    FlameAnim = rand()%2;
}

// --------------------------------------------------------------------------------------
// Den alten, billigen Blitz rendern
// --------------------------------------------------------------------------------------

void PlayerClass::DrawNormalLightning(int DrawLength)
{
    //----- Strahl des Blitzes anzeigen

    float l,  r,  o,  u;					// Vertice Koordinaten
    float tl, tr, to, tu;					// Textur Koordinaten
    float x, y;

    x = (float)(xpos - TileEngine.XOffset+60);		// Position errechnen
    y = (float)(ypos - TileEngine.YOffset+36);

    if (Blickrichtung == LINKS)
        x -= 56;

    l = x-0.5f;						// Links
    o = y-DrawLength*32-0.5f;		// Oben
    r = x+31+0.5f;					// Rechts
    u = y+31+0.5f;					// Unten

    tl = 0.0f;
    tr = 1.0f;
    to = 0.0f;
    tu = 1.0f * DrawLength;
    if (tu == 0.0f)
        tu =  1.0f;

    TriangleStrip[0].x		= l;		// Links oben
    TriangleStrip[0].y		= o;
    TriangleStrip[0].tu		= tl;
    TriangleStrip[0].tv		= to;

    TriangleStrip[1].x		= r;		// Rechts oben
    TriangleStrip[1].y		= o;
    TriangleStrip[1].tu		= tr;
    TriangleStrip[1].tv		= to;

    TriangleStrip[2].x		= l;		// Links unten
    TriangleStrip[2].y		= u;
    TriangleStrip[2].tu		= tl;
    TriangleStrip[2].tv		= tu;

    TriangleStrip[3].x		= r;		// Rechts unten
    TriangleStrip[3].y		= u;
    TriangleStrip[3].tu		= tr;
    TriangleStrip[3].tv		= tu;

    TriangleStrip[0].color = TriangleStrip[1].color = TriangleStrip[2].color = TriangleStrip[3].color = 0xFFFFFFFF;

    DirectGraphics.SetTexture(  Projectiles.Blitzstrahl[BlitzAnim].itsTexIdx );

    // Blitz rotieren lassen
    D3DXMATRIX	matRot, matTrans, matTrans2;

    D3DXMatrixRotationZ  (&matRot, DegreetoRad[int(BlitzWinkel)]);// Rotationsmatrix
    D3DXMatrixTranslation(&matTrans,  -x-16, -y-56, 0.0f);		// Transformation zum Ursprung

    if (Blickrichtung == RECHTS)
        D3DXMatrixTranslation(&matTrans2,  x-23,  y,    0.0f);		// Transformation wieder zur�ck
    else
        D3DXMatrixTranslation(&matTrans2,  x+31,  y,    0.0f);		// Transformation wieder zur�ck

    D3DXMatrixMultiply	 (&matWorld, &matTrans, &matRot);		// Verschieben und rotieren
    D3DXMatrixMultiply	 (&matWorld, &matWorld, &matTrans2);	// und wieder zur�ck
#if defined(PLATFORM_DIRECTX)
    lpD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
#elif defined(PLATFORM_SDL)
    g_matModelView = matWorld * g_matView;
#if defined(USE_GL1)
    load_matrix( GL_MODELVIEW, g_matModelView.data() );
#endif
#endif

    DirectGraphics.SetFilterMode (true);

    // Blitzstrahl zeichnen
    v1.color = v2.color = v3.color = v4.color = 0xFFFFFFFF;
    DirectGraphics.RendertoBuffer (D3DPT_TRIANGLESTRIP, 2, &TriangleStrip[0]);

    // nochmal rendern bei supershot?
    if (RiesenShotExtra > 0.0f)
        DirectGraphics.RendertoBuffer (D3DPT_TRIANGLESTRIP, 2, &TriangleStrip[0]);

    DirectGraphics.SetFilterMode (false);

    // Normale Projektions-Matrix wieder herstellen, wenn der Screen nicht gerade gewackelt wird
    if (WackelMaximum <= 0.0f)
    {
        D3DXMatrixRotationZ (&matWorld, 0.0f);
#if defined(PLATFORM_DIRECTX)
        lpD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
#elif defined(PLATFORM_SDL)
        g_matModelView = matWorld * g_matView;
#if defined(USE_GL1)
        load_matrix( GL_MODELVIEW, g_matModelView.data() );
#endif
#endif
    }
    else
    {
        int Winkel = int(WackelValue+500)-500;			// +500 und -500 damit er von -1.0 bis +1.0
        // nich stehen bleibt, weil -0.99 bis +0.99
        // auf 0 gerundet wird

        // Winkel angleichen, damit er immer zwischen 0� und 360� bleibt
        if (Winkel > 360) Winkel -= 360;
        if (Winkel < 0)	  Winkel += 360;
        D3DXMatrixRotationZ  (&matRot, DegreetoRad[Winkel]);

        if (WackelMaximum <= 0.0f)					// Wackeln zuende ?
            WackelMaximum = 0.0f;					// Dann aufh�ren damit

        // rotierte Matrix setzen
#if defined(PLATFORM_DIRECTX)
        lpD3DDevice->SetTransform(D3DTS_WORLD, &matRot);
#elif defined(PLATFORM_SDL)
        g_matModelView = matRot * g_matView;
#if defined(USE_GL1)
        load_matrix( GL_MODELVIEW, g_matModelView.data() );
#endif
#endif
    }

    DirectGraphics.SetColorKeyMode();
}

// --------------------------------------------------------------------------------------
// Den neuen, mega geilen Blitz rendern
// bestehend aus mehreren, zusammengesetzten vertice-strichen
// Drawlength gibt an, wie lang der Blitz ist
// --------------------------------------------------------------------------------------

void PlayerClass::DrawCoolLightning(int DrawLength, float mul)
{
    int r = 0;
    int zufall = 0;
    int ystrahl = 0;
    int xstrahl = 0;
    float xstart = 0.0f;
    float ystart = 0.0f;
    int yoff = 0;
    int size = 0;
    int maxintersections = 0;					// anzahl zwischenschritte im blitz
    D3DCOLOR col;

    ystrahl = 0;
    xstrahl = 0;

    xstart = (float)(18);
    ystart = (float)(4);

    maxintersections = DrawLength + 2;

    // Blitze neu zusammensetzen?
    if (Console.Showing == false)
        changecount -= 1.0f SYNC;

    if (changecount <= 0.0f)
    {
        changecount = (float)(rand()%10 + 1) / 10.0f;

        for (int n = 0; n < 12; n ++)
        {
            // Farbe festlegen, hell oder dunkel oder fast weiss
            zufall = rand()%6;
            if (zufall < 3)
            {
                r = rand()%64;
                col = D3DCOLOR_RGBA(r/2, r, rand()%32 + 224, rand()%64 + 192);
            }
            else if (zufall < 5)
            {
                r = rand()%32;
                col = D3DCOLOR_RGBA(r/2, r, rand()%24 + 64, rand()%24 + 192);
            }
            else
            {
                r = rand()%128+128;
                col = D3DCOLOR_RGBA(r, r, 255, rand()%16 + 128);
            }

            // Startpunkt auf 0 setzen
            yoff = 0;
            size = rand()%4 + 1;

            for (int i = 0; i < maxintersections * 2; i += 2)
            {
                // zwei neue Punkte zwischen letztem Punkt und Endpunkt per Zufall setzen
                xstrahl = (int)((xpos - TileEngine.XOffset + xstart) + (rand()%32 - 16) * mul);
                ystrahl = (int)(ypos - TileEngine.YOffset + ystart - yoff);

                // Am End- und Austrittspunkt geb�ndelt
                if (i == 0)
                {
                    xstrahl = (int)(xpos - TileEngine.XOffset + xstart) + rand()%6 - 2;
                    ystrahl = (int)(ypos - TileEngine.YOffset + ystart);
                }

                if (i >= (maxintersections - 1) * 2)
                {
                    xstrahl = (int)(xpos - TileEngine.XOffset + xstart) + rand()%6 - 2;
                    ystrahl = (int)(ypos - TileEngine.YOffset + ystart - (DrawLength + 1)*32);
                }

                // Position setzen
                strahlen[n][i + 0].x = (float)(xstrahl - size + xstart);
                strahlen[n][i + 1].x = (float)(xstrahl + size + xstart);
                strahlen[n][i + 0].y = (float)(ystrahl + ystart);
                strahlen[n][i + 1].y = (float)(ystrahl + ystart);

                // texturkoordinaten setzen
                strahlen[n][i + 0].tu = 0.0f;
                strahlen[n][i + 0].tv = 0.0f;
                strahlen[n][i + 1].tu = 1.0f;
                strahlen[n][i + 1].tv = 0.0f;

                // Farbe setzen
                strahlen[n][i + 0].color = col;
                strahlen[n][i + 1].color = col;

                yoff += rand()%21+24;
            }
        }
    }

    // Strahlen rendern
    DirectGraphics.SetTexture(  Projectiles.BlitzTexture.itsTexIdx );

    for (int n = 0; n < 12; n ++)
    {
        DirectGraphics.RendertoBuffer(D3DPT_TRIANGLESTRIP,
                                      (maxintersections - 1) * 2 - 1,
                                      &strahlen[n][0]);
    }

    return;
}

// --------------------------------------------------------------------------------------
// Blitz animieren und zeichnen
//
// Ist die Blitztaste schon lange genug gedr�ckt worden, wird der ganze Blitz angezeigt
// Andernfalls nur der Blitzursprung beim Spieler
// --------------------------------------------------------------------------------------

bool PlayerClass::DoLightning(void)
{
    int			DrawLength;			// L�nge des Blitze mit ber�cksichtigten W�nden im Weg
    GegnerClass  *pEnemy;			// F�r die Blitz/Gegner Kollision

    DrawLength = BlitzLength-1;		// Vom Maximum ausgehen (das wird sp�ter "gek�rzt")

    if (BlitzWinkel < 0)			// Einmal im Kreis rumgedreht ? Dann wieder
        BlitzWinkel += 360;			// von vorne beginnen mit der Rotation
    if (BlitzWinkel > 360)			// und zwar im und gegen den Uhrzeigersinn
        BlitzWinkel -= 360;

    // Ende des Blitzes beim Spieler leuchten lassen, falls er ihn grade noch aufl�dt

    float x, y;

    x = (float)(xpos - TileEngine.XOffset+35);		// Position errechnen
    y = (float)(ypos - TileEngine.YOffset+35);

    //if (Blickrichtung == LINKS)
    //	x -= 56;

    DirectGraphics.SetAdditiveMode();

//----- Blitz animieren

    if (Console.Showing == false)
        BlitzCount += SpeedFaktor;				// Counter erh�hen

    if (BlitzCount > PLAYER_BLITZ_SPEED)	// Animationsgrenze �berschritten ?
    {
        BlitzCount = 0.0f;					// Dann Counter wieder auf Null setzen und
        BlitzAnim++;						// N�chste Animationsphase
        if (BlitzAnim >= 4)					// Animation von vorne beginnen ?
            BlitzAnim = 0;
    }

//----- Wird der Blitz schon angezeigt ?
//----- wenn nicht, dann das Aufladen anzeigen

    float xstart = float(xpos+20);
    float ystart = float(ypos+21);

    //DKS - Support new trig sin/cos lookup table and use deg/rad versions of sin/cos:
    //xstart += float(36*cos(PI * (BlitzWinkel-90) / 180));
    //ystart += float(36*sin(PI * (BlitzWinkel-90) / 180));
	xstart += 36.0f*cos_deg(BlitzWinkel-90.0f);
	ystart += 36.0f*sin_deg(BlitzWinkel-90.0f);

    if (BlitzStart < PLAYER_BLITZ_START)
    {
        D3DCOLOR Color, Color2;

        if (BlitzStart < PLAYER_BLITZ_START)
        {
            Color  = D3DCOLOR_RGBA(255, 255, 255, int(BlitzStart * 25));
            Color2 = D3DCOLOR_RGBA(255, 255, 255, int(BlitzStart * 25) / 5);
        }
        else
        {
            Color  = D3DCOLOR_RGBA(255, 255, 255, 255);
            Color2 = D3DCOLOR_RGBA(255, 255, 255, 48);
        }

        Projectiles.Blitzflash[BlitzAnim].RenderSprite(float(xstart-18-TileEngine.XOffset),
                                           float(ystart-18-TileEngine.YOffset), Color);

        // noch glow um die blitzenden?
        if (options_Detail >= DETAIL_HIGH)
            Projectiles.Blitzflash[3-BlitzAnim].RenderSpriteScaled(float(xstart-58-TileEngine.XOffset),
                    float(ystart-58-TileEngine.YOffset), 144, 144, 0, Color2);

        DirectGraphics.SetColorKeyMode();
        return true;
    }

//----- Blitz anzeigen - zuerst wird festgestellt, wie lang der Blitz ist,
//		d.h., ob er eine Wand getroffen hat

    // Anfang des Blitzes leuchten lassen
    Projectiles.Blitzflash[BlitzAnim].RenderSprite(float(xstart-18-TileEngine.XOffset),
                                       float(ystart-18-TileEngine.YOffset), 0xFFFFFFFF);

    // noch glow um die blitzenden?
    if (options_Detail >= DETAIL_HIGH)
    {
        Projectiles.Blitzflash[3-BlitzAnim].RenderSpriteScaled(float(xstart-58-TileEngine.XOffset),
                float(ystart-58-TileEngine.YOffset), 144, 144, 0, 0x30FFFFFF);
    }

    // Startpunkt der Kollisionsabfrage auch schon mit ein wenig Abstand zum Spieler
    //xstart -= float(20*cos(PI * (BlitzWinkel-90) / 180));
    //ystart -= float(20*sin(PI * (BlitzWinkel-90) / 180));
	xstart -= 20.0f*cos_deg(BlitzWinkel-90);
	ystart -= 20.0f*sin_deg(BlitzWinkel-90);

    RECT	Rect;							// Rechteck f�r die Kollisionserkennung
    // ein Blitz-St�ck wird grob durch
    Rect.left   = 0;						// ein 32x32 Rechteck abgesch�tzt
    Rect.top    = 0;
    Rect.right  = 31;
    Rect.bottom = 31;

    float xs, ys;

    // Rechtecke f�r die Kollisionsabfrage rotieren lassen
    for (int i=0; i<BlitzLength+1; i++)
    {
        //DKS - #ifdef'd this check
#ifdef _DEBUG
        // Zum anzeigen der Rects, die gepr�ft werden
        if (DebugMode == true)
            RenderRect(float(xstart-TileEngine.XOffset),
                       float(ystart-TileEngine.YOffset),
                       31, 31, 0x80FFFFFF);
#endif //DEBUG

        xs = xstart;
        ys = ystart;

        //DKS - Lightmaps have been disabled (never worked originally, see Tileengine.cpp's
        //      comments for DrawLightmap())
        // Blitz-LightMap rendern
        //if (options_Detail >= DETAIL_MEDIUM)
        //    TileEngine.DrawLightmap(LIGHTMAP_BLITZ, xs + 16, ys + 16, 255);

        // Blitz auf Kollision mit den Gegnern pr�fen
        pEnemy = Gegner.pStart;			// Anfang der Gegnerliste
        while (pEnemy != NULL)				// Noch nicht alle durch ?
        {
            if (pEnemy->Active == true &&		// Ist der Gegner �berhaupt aktiv ?
                    pEnemy->Destroyable == true)	// und zerst�rbar ?

            {
                // �berschneiden sich die Rechtecke ?
                if (xstart + 31 > pEnemy->xPos &&
                        xstart < pEnemy->xPos + GegnerRect[pEnemy->GegnerArt].right &&
                        ystart + 31 > pEnemy->yPos &&
                        ystart < pEnemy->yPos + GegnerRect[pEnemy->GegnerArt].bottom)
                {
                    // Funken spr�hen
                    if (BlitzCount == 0.0f && BlitzAnim%2 == 0)
                        PartikelSystem.PushPartikel(xs+12, ys+12, LASERFUNKE);

                    // Gegner blinken lassen
                    pEnemy->DamageTaken = 255;

                    // Energy abziehen
                    pEnemy->Energy = pEnemy->Energy - BLITZ_POWER SYNC;

                    // Hit Sound
                    //DKS - Added function WaveIsPlaying() to SoundManagerClass:
                    if (!SoundManager.WaveIsPlaying(SOUND_HIT + pEnemy->HitSound))
                        SoundManager.PlayWave(100, 128, 21000 + rand()%1000, SOUND_HIT + pEnemy->HitSound);

                    // PlattForm ShootButton getroffen ? Dann Blitz k�rzen
                    //
                    if (pEnemy->GegnerArt == SHOOTBUTTON)
                    {
                        if (BlitzCount == 0.0f && BlitzAnim%2 == 0)
                            PartikelSystem.PushPartikel(xs+12, ys+12, LASERFUNKE);	// Funken spr�hen
                        DrawLength = i-1;											// Blitz "k�rzen"
                        i = BlitzLength + 2;
                        break;														// Und Schleife verlassen
                    }
                }
            }

            pEnemy = pEnemy->pNext;			// N�chsten Gegner testen
        }

        // Zerst�rbare W�nde ?
        TileEngine.CheckDestroyableWalls(xs,  ys, 0, 0, Rect);
        TileEngine.BlockDestroyRechts(xs, ys, xs, ys, Rect);
        TileEngine.BlockDestroyLinks (xs, ys, xs, ys, Rect);
        TileEngine.BlockDestroyOben  (xs, ys, xs, ys, Rect);
        TileEngine.BlockDestroyUnten (xs, ys, xs, ys, Rect);

        if (TileEngine.BlockLinks (xs, ys, xs, ys, Rect) & BLOCKWERT_WASSER &&	// Im Wasser blitzen ?
                TileEngine.BlockRechts(xs, ys, xs, ys, Rect) & BLOCKWERT_WASSER &&
                TileEngine.BlockOben  (xs, ys, xs, ys, Rect) & BLOCKWERT_WASSER &&
                TileEngine.BlockUnten (xs, ys, xs, ys, Rect) & BLOCKWERT_WASSER &&
                rand()%80 == 0)
            PartikelSystem.PushPartikel(xs+rand()%32, ys+rand()%32, BUBBLE);		// Dann blubbern


        if (TileEngine.BlockLinks (xs, ys, xs, ys, Rect) & BLOCKWERT_WAND  ||	// Eine Wand getroffen ?
                TileEngine.BlockRechts(xs, ys, xs, ys, Rect) & BLOCKWERT_WAND  ||
                TileEngine.BlockOben  (xs, ys, xs, ys, Rect) & BLOCKWERT_WAND  ||
                TileEngine.BlockUnten (xs, ys, xs, ys, Rect) & BLOCKWERT_WAND)
        {
            if (BlitzCount == 0.0f && BlitzAnim%2 == 0)
                PartikelSystem.PushPartikel(xs+12, ys+12, LASERFUNKE);	// Funken spr�hen
            //DKS - Support new trig sin/cos lookup table and use deg/rad versions of sin/cos:
            //xstart += float(32*cos(PI * (BlitzWinkel-90) / 180));
            //ystart += float(32*sin(PI * (BlitzWinkel-90) / 180));
			xstart += 32.0f*cos_deg(BlitzWinkel-90);
			ystart += 32.0f*sin_deg(BlitzWinkel-90);
            DrawLength = i-1;										// Blitz "k�rzen"
            break;													// und Schleife verlassen
        }

        //DKS - Support new trig sin/cos lookup table and use deg/rad versions of sin/cos:
        //xstart += float(32*cos(PI * (BlitzWinkel-90) / 180));
        //ystart += float(32*sin(PI * (BlitzWinkel-90) / 180));
		xstart += 32.0f*cos_deg(BlitzWinkel-90);
		ystart += 32.0f*sin_deg(BlitzWinkel-90);
    }

    // Position f�r das Ende des Blitzes wieder ein wenig zur�ckverschieben
    //DKS - Support new trig sin/cos lookup table and use deg/rad versions of sin/cos:
    //xstart -= float(16*cos(PI * (BlitzWinkel-90) / 180));
    //ystart -= float(16*sin(PI * (BlitzWinkel-90) / 180));
	xstart -= 16.0f*cos_deg(BlitzWinkel-90);
	ystart -= 16.0f*sin_deg(BlitzWinkel-90);

    // Ende des Blitzes leuchten lassen
    Projectiles.Blitzflash[BlitzAnim].RenderSprite(float(xstart-18-TileEngine.XOffset),
                                       float(ystart-18-TileEngine.YOffset), 0xFFFFFFFF);

    // noch glow um die blitzenden?
    if (options_Detail >= DETAIL_HIGH)
    {
        Projectiles.Blitzflash[3-BlitzAnim].RenderSpriteScaled(float(xstart-58-TileEngine.XOffset),
                float(ystart-58-TileEngine.YOffset), 144, 144, 0, 0x30FFFFFF);
    }

    // Blitz rotieren lassen
    D3DXMATRIX	matRot, matTrans, matTrans2;

    D3DXMatrixRotationZ  (&matRot, DegreetoRad[int(BlitzWinkel)]);// Rotationsmatrix
    D3DXMatrixTranslation(&matTrans, -x, -y, 0.0f);		// Transformation zum Ursprung
    if (Blickrichtung == RECHTS)
        D3DXMatrixTranslation(&matTrans2, x, y, 0.0f);		// Transformation wieder zur�ck
    else
        D3DXMatrixTranslation(&matTrans2, x, y, 0.0f);		// Transformation wieder zur�ck
    D3DXMatrixMultiply	 (&matWorld, &matTrans, &matRot);		// Verschieben und rotieren
    D3DXMatrixMultiply	 (&matWorld, &matWorld, &matTrans2);	// und wieder zur�ck
#if defined(PLATFORM_DIRECTX)
    lpD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
#elif defined(PLATFORM_SDL)
    g_matModelView = matWorld * g_matView;
#if defined(USE_GL1)
    load_matrix( GL_MODELVIEW, g_matModelView.data() );
#endif
#endif

    DirectGraphics.SetFilterMode (true);
    if (DrawLength < 0)
        DrawLength = 0;

    if (options_Detail >= DETAIL_HIGH)
        DrawCoolLightning(DrawLength);
    else
        DrawNormalLightning(DrawLength);

    // nochmal rendern bei supershot?
    if (RiesenShotExtra > 0.0f)
    {
        if (options_Detail >= DETAIL_HIGH)
            DrawCoolLightning(DrawLength, 1.2f);
        else
            DrawNormalLightning(DrawLength);
    }

    DirectGraphics.SetFilterMode (false);

    // Normale Projektions-Matrix wieder herstellen, wenn der Screen nicht gerade gewackelt wird
    if (WackelMaximum <= 0.0f)
    {
        D3DXMatrixRotationZ (&matWorld, 0.0f);
#if defined(PLATFORM_DIRECTX)
        lpD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
#elif defined(PLATFORM_SDL)
        g_matModelView = matWorld * g_matView;
#if defined(USE_GL1)
        load_matrix( GL_MODELVIEW, g_matModelView.data() );
#endif
#endif
    }
    else
    {
        int Winkel = int(WackelValue+500)-500;			// +500 und -500 damit er von -1.0 bis +1.0
        // nich stehen bleibt, weil -0.99 bis +0.99
        // auf 0 gerundet wird

        // Winkel angleichen, damit er immer zwischen 0� und 360� bleibt
        if (Winkel > 360) Winkel -= 360;
        if (Winkel < 0)	  Winkel += 360;
        D3DXMatrixRotationZ  (&matRot, DegreetoRad[Winkel]);

        if (WackelMaximum <= 0.0f)					// Wackeln zuende ?
            WackelMaximum = 0.0f;					// Dann aufh�ren damit

        // rotierte Matrix setzen
#if defined(PLATFORM_DIRECTX)
        lpD3DDevice->SetTransform(D3DTS_WORLD, &matRot);
#elif defined(PLATFORM_SDL)
        g_matModelView = matWorld * g_matView;
#if defined(USE_GL1)
        load_matrix( GL_MODELVIEW, g_matModelView.data() );
#endif
#endif
    }


    DirectGraphics.SetColorKeyMode();

    return true;
}

// --------------------------------------------------------------------------------------
// BeamBlitz aufladen
//
// Der Beam l�dt sich langsam auf, je nach Blitzl�nge unterschiedlich schnell
// --------------------------------------------------------------------------------------

bool PlayerClass::LoadBeam (void)
{
    if (BlitzWinkel < 0)			// Einmal im Kreis rumgedreht ? Dann wieder
        BlitzWinkel += 360;			// von vorne beginnen mit der Rotation
    if (BlitzWinkel > 360)			// und zwar im und gegen den Uhrzeigersinn
        BlitzWinkel -= 360;

    //----- Blitz animieren

    BlitzCount += SpeedFaktor;				// Counter erh�hen
    if (BlitzCount > PLAYER_BLITZ_SPEED)	// Animationsgrenze �berschritten ?
    {
        BlitzCount = 0.0f;					// Dann Counter wieder auf Null setzen und
        BlitzAnim++;						// N�chste Animationsphase
        if (BlitzAnim >= 4)					// Animation von vorne beginnen ?
            BlitzAnim = 0;
    }

    // Ende des Blitzes beim Spieler leuchten lassen, falls er ihn grade noch aufl�dt
    //
    float x;
    //float y;

    x = (float)(xpos - TileEngine.XOffset+60);		// Position errechnen
    //y = (float)(ypos - TileEngine.YOffset+36);

    if (Blickrichtung == LINKS)
        x -= 56;

    DirectGraphics.SetAdditiveMode();

//----- Wird der Blitz schon angezeigt ?
//----- wenn nicht, dann das Aufladen anzeigen

    D3DCOLOR Color;
    int		 a;

    a = int(BlitzStart * 255 / 160);

    if (BlitzStart < PLAYER_BEAM_MAX)
        Color = D3DCOLOR_RGBA(255, 255, 255, a);
    else
        Color = D3DCOLOR_RGBA(255, 255, 255, 255);

    float	xstart = float(xpos+20);
    float	ystart = float(ypos+21);

    //DKS - Support new trig sin/cos lookup table and use deg/rad versions of sin/cos:
    //xstart += float(28*cos(PI * (BlitzWinkel-90) / 180));
    //ystart += float(28*sin(PI * (BlitzWinkel-90) / 180));
	xstart += 28.0f*cos_deg(BlitzWinkel-90);
	ystart += 28.0f*sin_deg(BlitzWinkel-90);

    // Ende des Blitzes leuchten lassen
    Projectiles.Blitzflash[BlitzAnim].RenderSprite(float(xstart-18-TileEngine.XOffset),
                                       float(ystart-18-TileEngine.YOffset), Color);

    BeamX = xstart+12;
    BeamY = ystart+12;

    // Auflade Partikel erzeugen?
    //
    if (BlitzStart < PLAYER_BEAM_MAX)
    {
        BeamCount -= float (1.0 SYNC);

        if (BeamCount < 0.0f)
        {
            BeamCount = 0.1f;

            int j = rand ()%360;
            //DKS - pretty obviously a bug, they mean to convert to degrees before calling sin (which takes radians)
            //      When I fixed this, I went ahead and added support for trig lookup table, and support for 
            //      rad/deg versions of sin/cos
            //PartikelSystem.PushPartikel (float (BeamX + sin ((float)j) * 50),
            //                               float (BeamY + cos ((float)j) * 50), BEAMSMOKE2, this);
			PartikelSystem.PushPartikel (BeamX + sin_deg(j) * 50.0f,
										   BeamY + cos_deg(j) * 50.0f, BEAMSMOKE2, this);
        }
    }

    DirectGraphics.SetColorKeyMode();

    return true;
}

// --------------------------------------------------------------------------------------
// Spieler erleidet Schaden
// ist Override == true dann auch mit Shield oder als Rad
// --------------------------------------------------------------------------------------

void PlayerClass::DamagePlayer (float Ammount,
                                bool Override)
{
    if (GodMode		 == true  ||
            CanBeDamaged == false ||
            (Shield > 0.0f && Override == false))
        return;

    // Skill Level ber�cksichtigen
    if (Skill == 0) Ammount *= 0.3f;
    if (Skill == 1) Ammount *= 0.6f;
    if (Skill == 2) Ammount *= 1.0f;
    if (Skill == 3) Ammount *= 2.2f;

    // Sound starten
    //DKS - Added function WaveIsPlaying() to SoundManagerClass:
    if (DamageCounter == 0.0f   &&
            Handlung != RADELN		&&
            Handlung != RADELN_FALL &&
            !SoundManager.WaveIsPlaying(SOUND_ABZUG + SoundOff))
        SoundManager.PlayWave(100, 128, 11025, SOUND_ABZUG + SoundOff);

    // Nur verwunden, wenn kein Rad, denn als Rad ist man unverwundbar
    if ((Handlung != RADELN &&
            Handlung != RADELN_FALL) ||
            Override == true)
    {
        Energy -= Ammount;
        if (Energy < 0.0f)
            Energy = 0.0f;

        DamageCounter = 5.0f;

        DrawPlayer(true, false);
        WasDamaged = true;
    }
}

// --------------------------------------------------------------------------------------
// Level zentrieren, damit der Spieler wieder im Mittelpunkt steht
// --------------------------------------------------------------------------------------

void PlayerClass::CenterLevel(void)
{
    TileEngine.XOffset = xpos - 300;
    TileEngine.YOffset = ypos - 280;
}

// --------------------------------------------------------------------------------------
// Waffenlevels f�r die n�chste Stufe berechnen
// --------------------------------------------------------------------------------------

void PlayerClass::CalcWeaponLevels(void)
{
    /*
    	for (int i = 0; i < 3; i++)
    		pPlayer->NextWeaponLevel[i] = 3 + (pPlayer->CurrentWeaponLevel[i] - 1) * 2;

    	pPlayer->NextWeaponLevel[3] = 3;
    */
}

// --------------------------------------------------------------------------------------
// Position der Schussflamme errechnen
// --------------------------------------------------------------------------------------

void PlayerClass::CalcFlamePos (void)
{
    float xoff, yoff;

    // auf dem FlugSack reiten
    if (Handlung == SACKREITEN)
    {
        yoff = 1;
        if (Blickrichtung == RECHTS)
            xoff =  80;
        else
            xoff = -35;
    }

    //DKS - This appears never to have been implemented (no image for it) so disabled it:
#if 0
    // Surfen
    else if (Handlung == SURFEN ||
             Handlung == SURFENJUMP)
    {
        xoff = 52;
        yoff = -14;
    }

    else if (Handlung == SURFENCROUCH)
    {
        xoff = 51;
        yoff = -2;
    }
#endif //0

    // alle anderen Handlugen
    else
    {
        if (Handlung == DUCKEN)
            yoff = 23;
        else
            yoff = 0;

        if (Blickrichtung == RECHTS)
            xoff =  66;
        else
            xoff = -33;
    }

    // X-Offset richtig berechnen
    xoff -= (int)(TileEngine.XOffset);
    yoff -= (int)(TileEngine.YOffset);

    // Und dann je nach Blickrichtung die Flamme und den Flare dazu setzen
    if (Blickrichtung == RECHTS)
    {
        if (Handlung == STEHEN   ||
                Handlung == SACKREITEN ||
                Handlung == DUCKEN)
        {
            AustrittAnim = 0;
            AustrittX = xoff - 10;
            AustrittY = yoff + 20;
        }
        else

            // im Sprung?
            if (Handlung == SPRINGEN)
            {
                // dabei hochzielen?
                if (Aktion[AKTION_OBEN])
                {
                    // nur noch?
                    if (!(Aktion[AKTION_LINKS] ||
                            Aktion[AKTION_RECHTS]))
                    {
                        AustrittAnim = 2;
                        AustrittX = xoff - 47;
                        AustrittY = yoff - 34;
                    }

                    // oder schr�g?
                    else
                    {
                        AustrittAnim = 1;
                        AustrittX = xoff - 21;
                        AustrittY = yoff - 12;
                    }
                }

                // normal springen
                else
                {
                    AustrittAnim = 0;
                    AustrittX = xoff - 10;
                    AustrittY = yoff + 15;
                }
            }

            else if (Handlung == LAUFEN)
            {
                AustrittAnim = 0;
                AustrittX = xoff - 2;
                AustrittY = yoff + 19;
            }

            else if (Handlung == SCHIESSEN_RO)
            {
                //im Stehen?
                if (WalkLock == true)
                {
                    AustrittAnim = 1;
                    AustrittX = xoff - 25;
                    AustrittY = yoff - 8;
                }

                // oder Laufen?
                else
                {
                    AustrittAnim = 1;
                    AustrittX = xoff - 21;
                    AustrittY = yoff - 12;
                }
            }

            else if (Handlung == SCHIESSEN_O)
            {
                AustrittAnim = 2;
                AustrittX = xoff - 49;
                AustrittY = yoff - 33;
            }
    }
    else
    {
        if (Handlung == STEHEN   ||
                Handlung == SACKREITEN   ||
                Handlung == DUCKEN)
        {
            AustrittAnim = 0;
            AustrittX = xoff + 10;
            AustrittY = yoff + 20;
        }
        else

            // im Sprung?
            if (Handlung == SPRINGEN)
            {
                // dabei hochzielen?
                if (Aktion[AKTION_OBEN])
                {
                    // nur noch?
                    if (!(Aktion[AKTION_LINKS] ||
                            Aktion[AKTION_RECHTS]))
                    {
                        AustrittAnim = 2;
                        AustrittX = xoff + 62;
                        AustrittY = yoff - 34;
                    }

                    // oder schr�g?
                    else
                    {
                        AustrittAnim = 1;
                        AustrittX = xoff + 21;
                        AustrittY = yoff - 12;
                    }
                }

                // normal springen
                else
                {
                    AustrittAnim = 0;
                    AustrittX = xoff + 6;
                    AustrittY = yoff + 15;
                }
            }

            else if (Handlung == LAUFEN)
            {
                AustrittAnim = 0;
                AustrittX = xoff;
                AustrittY = yoff + 19;
            }

            else if (Handlung == SCHIESSEN_LO)
            {
                //im Stehen?
                if (WalkLock == true)
                {
                    AustrittAnim = 1;
                    AustrittX = xoff + 21;
                    AustrittY = yoff - 8;
                }

                // oder im Laufen
                else
                {
                    AustrittAnim = 1;
                    AustrittX = xoff + 8;
                    AustrittY = yoff - 10;
                }
            }
            else if (Handlung == SCHIESSEN_O)
            {
                AustrittAnim = 2;
                AustrittX = xoff + 62;
                AustrittY = yoff - 33;
            }
    }
}

// --------------------------------------------------------------------------------------
// Austrittspunkt des Schusses errechnen
// --------------------------------------------------------------------------------------

void PlayerClass::CalcAustrittsPunkt(void)
{
    //float xver = 0.0f;	// Verschiebung des Austrittspunktes, wenn man vor einer Wandsteht

    //----- Genauen Pixel am Anfang derFlamme finden
    //
    CalcFlamePos();
    AustrittX += (float)TileEngine.XOffset;
    AustrittY += (float)TileEngine.YOffset;
    switch (AustrittAnim)
    {
    // gerade flamme
    case 0:
    {
        if (Blickrichtung == LINKS)
            AustrittX += 34;
        else
            AustrittX += 4;

        AustrittY += 24 / 2 - 1;
    }
    break;

    // schr�g oben
    case 1:
    {
        if (Blickrichtung == LINKS)
            AustrittX += 32;
        else
            AustrittX += 8;

        AustrittY += 26;
    }
    break;

    // oben
    case 2:
    {
        AustrittX += 24 / 2;
        AustrittY += 38;
    }
    break;

    case 3:
    {
        AustrittX += 40 / 2;
        AustrittY += 36 / 2;
    }
    break;
    }
}

// --------------------------------------------------------------------------------------
// Spieler aus dem Level rausgelaufen?
// --------------------------------------------------------------------------------------

bool PlayerClass::CheckLevelExit(void)
{
    // Spieler aus Level draussen?
    if (xpos + Player[0].CollideRect.right  < TileEngine.XOffset	    ||
            xpos + Player[0].CollideRect.left   > TileEngine.XOffset + 640 ||
            ypos + Player[0].CollideRect.bottom < TileEngine.YOffset	    ||
            ypos + Player[0].CollideRect.top    > TileEngine.YOffset + 480)
        return true;

    return false;
}

// --------------------------------------------------------------------------------------
// Spieler im Ausgang
// --------------------------------------------------------------------------------------

void PlayerClass::PlayerInExit(void)
{
    StageClearRunning = false;

    // Summary Screen zeigen, wenn nicht im Tutorial Mode
    if (RunningTutorial == false)
        SummaryScreen();

    Stage = NewStage;		// Anpassen dass wir im neuen Level sind

    SpielZustand = MAINMENU;
    Console.Hide();

    // Tutorial Level zuende? Dann normal zur�ck ins Hauptmenu
    //

    if (Stage == TUTORIAL_END)
    {
        NewStage = -1;
        Stage    = -1;
        pMenu->AktuellerZustand = MENUZUSTAND_MAINMENU;
        GUI.HideBoxFast();
        SoundManager.StopSong(MUSIC_STAGECLEAR, false);
    }

    // Alle Levels durch? Dann Outtro starten
    //

    else if (Stage > MAX_LEVELS)
        StartOuttro();

    // Ansonsten Savegame Menu
    //

    else if (RunningTutorial == false)
    {
        pMenu->LoadSavegames();					// Slots mit aktuellen Savegames f�llen
        pMenu->AktuellerPunkt = 0;
        pMenu->AktuellerZustand = MENUZUSTAND_SAVEGAME;
    }
    else
    {
        for (int p = 0; p < NUMPLAYERS; p++)
            Player[p].GodMode = false;

        Grenades = 3;
        PowerLines = 3;
        SmartBombs = 1;

        RunningTutorial = false;
        InitNewGameLevel(NewStage);// Neues level laden
    }

    RunningTutorial = false;
}

// --------------------------------------------------------------------------------------
// Spieler l�uft grad ins Exit
// --------------------------------------------------------------------------------------

void PlayerClass::RunPlayerExit(void)
{
    //DKS - Added support for non-looped music, so this is no longer necessary..
    //      This was causing stage-clear music to never be heard in the SDL port.
    //// Musik zuende ?
    //if (MUSIC_IsFinished(SoundManager.its_Songs[MUSIC_STAGECLEAR]->SongData))
    //    SoundManager.StopSong(MUSIC_STAGECLEAR, false);

    if (CheckLevelExit() == true)
        PlayerInExit();
}

// --------------------------------------------------------------------------------------
// Alle Spieler auf einem Flugsack?
// --------------------------------------------------------------------------------------

bool PlayerClass::Riding(void)
{
    for (int p = 0; p < NUMPLAYERS; p++)
        if (Player[p].Handlung != SACKREITEN &&
                Player[p].Handlung != DREHEN &&
                Player[p].Handlung != TOT)
            return false;

    return true;
}

// --------------------------------------------------------------------------------------
// Level bei Flugsack Passage scrollen
// --------------------------------------------------------------------------------------

void PlayerClass::ScrollFlugsack(void)
{
    bool BeideFrei = true;

    for (int p = 0; p < NUMPLAYERS; p++)
        if (Player[p].FesteAktion > -1)
            BeideFrei = false;
    if (!FlugsackFliesFree &&
            Riding() &&
            BeideFrei == true)
        TileEngine.YOffset -= (float)(PLAYER_FLUGSACKSPEED SYNC);
}

// --------------------------------------------------------------------------------------
// Waffen durchcyclen
// --------------------------------------------------------------------------------------

void PlayerClass::checkWeaponSwitch(void)
{
    if (!Aktion[AKTION_WAFFEN_CYCLE])
        weaponswitchlock = false;

    if (Aktion[AKTION_WAFFEN_CYCLE] &&
            !weaponswitchlock)
    {
        weaponswitchlock = true;

        do
        {
            SelectedWeapon++;
        }
        while (CurrentWeaponLevel[SelectedWeapon] == 0);

        if (SelectedWeapon >= 3)
            SelectedWeapon = 0;
    }
}

//DKS - Player sprites are no longer static globals, but instead class member vars:
//      This function will load the sprites specific to each player. If
//      player_num is 0, it will load Player 1's sprites, or Player 2's otherwise.
void PlayerClass::LoadSprites()
{
    if (SpritesLoaded)
        return;

    SpritesLoaded = true;
    std::string prefix;

    if (PlayerNumber == 1)
        prefix = "p2_";
    else
        prefix = "p1_";

    // Idle
    PlayerIdle.LoadImage( prefix + "hurri_idle.png",  350, 320, 70, 80, 5, 4);

    // Kucken
    PlayerKucken.LoadImage( prefix + "hurri_kucken.png",  140, 80, 70, 80, 2, 1);

    // Umkucken
    PlayerIdle2.LoadImage( prefix + "hurri_idleumkuck.png",  980,  320, 70, 80, 14, 4);

    // Laufen
    PlayerRun.LoadImage( prefix + "hurri_laufen.png",  350,  320, 70, 80, 5, 4);

    // Diagonal schauen/schiessen
    PlayerDiagonal.LoadImage( prefix + "hurri_shootdiagonal.png",  140,  80, 70, 80, 2, 1);

    // Hoch schauen/schiessen
    PlayerOben.LoadImage( prefix + "hurri_shootup.png",  140,  80, 70, 80, 2, 1);

    // Ducken
    PlayerCrouch.LoadImage( prefix + "hurri_crouch.png",  140,  80, 70, 80, 2, 1);

    // Springen normal, diagonal und nach oben
    PlayerJump.LoadImage( prefix + "hurri_jump.png",  280,  240, 70, 80, 4, 3);
    PlayerJumpDiagonal.LoadImage( prefix + "hurri_jumpschraeg.png",  280,  240, 70, 80, 4, 3);
    PlayerJumpUp.LoadImage( prefix + "hurri_jumpup.png",  280,  240, 70, 80, 4, 3);

    // Rundumschuss
    PlayerBlitz.LoadImage( prefix + "hurri_rundumschuss.png", 490, 480, 70, 80, 7, 6);

    // Pissen
    PlayerPiss.LoadImage( prefix + "hurri_pissen.png", 490, 240, 70, 80, 7, 3);

    // Flugsack
    PlayerRide.LoadImage( prefix + "hurri_ride.png",   450, 480, 90,120, 5, 4);

    // Stachelrad
    PlayerRad.LoadImage( prefix + "hurri_rad.png",    140,  70, 35, 35, 4, 2);
}
