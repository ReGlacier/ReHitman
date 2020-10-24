#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZWinEvents.h>
#include <Glacier/ZMath.h>
#include <Glacier/ZRTTI.h>
#include <Glacier/REFTAB32.h>
#include <Glacier/LINKREFTAB.h>
#include <Glacier/LINKSORTREFTAB.h>

#include <BloodMoney/Game/UI/UIFWD.h>
#include <BloodMoney/Game/UI/ZWinInput.h>

namespace Hitman::BloodMoney
{
    class ZWINDOWS
    {
    public:
        class EventSubscriber
        {
        public:
            int32_t m_groupObjectId; //0x0000
            int32_t m_chkval; //0x0004 (always 0x20000)
            int32_t m_eventID; //0x0008
            bool m_flag_0C; //0x000C
            bool m_field_0D; //0x000D
            bool m_field_0E; //0x000E
            bool m_field_0F; //0x000F
            float fArg; //0x0010
            int32_t m_unkPtr; //0x0014

// *** FIELDS BELOW NOT CONFIRMED BY REVERSE ***
//            int32_t m_field18; //0x0018
//            int32_t m_field1C; //0x001C
//            int32_t m_field20; //0x0020
//            int32_t m_field24; //0x0024
//            int32_t m_field28; //0x0028
//            int32_t m_field2C; //0x002C
//            float m_field30; //0x0030
//            int32_t m_field34; //0x0034
//            int32_t m_unknPtr2; //0x0038
//            char pad_003C[168]; //0x003C
        }; //Size: 0x00E4

        typedef bool(Glacier::ZGEOM::* AnimCallback_t)(Glacier::Animation::ActiveAnimation*, float, float, Glacier::uint);

        /// === vftable ===
        virtual void Release(bool);                                             //#0
        virtual void PreSave(Glacier::ISerializerStream &);                     //#1
        virtual void PostSave(Glacier::ISerializerStream &);                    //#2
        virtual void PreLoad(Glacier::ISerializerStream &);                     //#3
        virtual void PostLoad(Glacier::ISerializerStream &);                    //#4
        virtual void PostProcess(Glacier::uint,Glacier::uint);                  //#5
        virtual void LoadSave(Glacier::ISerializerStream &,bool);               //#6
        virtual void LoadObject(Glacier::IInputSerializerStream &);             //#7
        virtual void SaveObject(Glacier::IOutputSerializerStream &);            //#8
        virtual void ExchangeObject(Glacier::ISerializerStream &);              //#9
        virtual void SetToDefault();                                            //#10
        virtual int GetTypeID();                                                //#11
        virtual void* GetProperties();                                          //#12
        virtual int GetObjectId();                                              //#13
        virtual void GetObjectIdAndMask(Glacier::uint &,Glacier::uint &);       //#14
        virtual Glacier::ZRTTI* GetOldClassInfo();                              //#15
        virtual void DoInit();                                                  //#16
        virtual int CalcCenSize();                                              //#17
        virtual void PreSaveGame();                                             //#18
        virtual void CheckPointSave(Glacier::ZCheckPointBuffer &);              //#19
        virtual void CheckPointLoad(Glacier::ZCheckPointBuffer &);              //#20
        virtual void Activate(bool);                                            //#21
        virtual void MakeInactive();                                            //#22
        virtual void MakeActive();                                              //#23
        virtual int  GetBoundTreeType();                                        //#24
        virtual void AnimCallBackToId(AnimCallback_t callback);                 //#25
        virtual void AnimCallBackFromId(int);                                   //#26
        virtual void CameraMessages(bool);                                      //#27
        virtual void* CreateExData();                                           //#28
        virtual void FreeExData();                                              //#29
        virtual void CopyExData(Glacier::ZGEOM const*);                         //#30
        virtual void GetChunkData(int);                                         //#31
        virtual void RecurGetNext(Glacier::ZBaseGeom const**);                  //#32
        virtual void DisableParentBoundAdjust();                                //#33
        virtual void ExpandBounds(float *,float *,float *,Glacier::ZBaseGeom *);//#34
        virtual void SetMat(float const*);                                      //#35
        virtual void SetPos(float const*);                                      //#36
        virtual void SetMatPos(float const*,float const*);                      //#37
        virtual void SetWorldPosition(float const*);                            //#38
        virtual void SetRootTM(float const*,float const*);                      //#39
        virtual void Display(bool);                                             //#40
        virtual void Select(bool);                                              //#41
        virtual void Hide(bool);                                                //#42
        virtual void Unknown_Wrong_NullFunction_0000();                         //#43
        virtual void HideRecursive(bool);                                       //#44
        virtual void Freeze(bool);                                              //#45
        virtual void DispBound(bool);                                           //#46
        virtual void SetMoving(bool);                                           //#47
        virtual void RequestCustomDraw();                                       //#48
        virtual bool HasOwnerDraw();                                            //#49
        virtual void OwnerDraw(Glacier::ZBaseGeom *, Glacier::uint,
                               Glacier::ZDrawBuffer*, Glacier::ZCAMERA*,
                               float const*, float const*, void const*);        //#50
        virtual void CorrectOwnerDrawMatrix(float *,float *,
                                            Glacier::ZBaseGeom*, Glacier::uint);//#51
        virtual void CorrectOwnerDrawPartMatrix(Glacier::ZMat3x3 *,
                                                Glacier::ZBaseGeom *,
                                                Glacier::ZBone *);              //#52
        virtual bool WantDrawBufferControl();                                   //#53
        virtual void DrawUpdate();                                              //#54
        virtual void DrawBufferViewUpdate(Glacier::ZDrawBuffer *,
                                          Glacier::ZCameraSpace *);             //#55
        virtual void DrawBufferViewUpdate(Glacier::ZDrawBuffer *,
                                          Glacier::ZBaseGeom *,Glacier::uint);  //#56
        virtual void WantViewPrimHideUnhideRequest();                           //#57
        virtual void WantViewPrimHide(Glacier::uchar,bool);                     //#58
        virtual void WantViewPrimHideMirrors(Glacier::uchar);                   //#59
        virtual void WantViewUseAllLOD();                                       //#60
        virtual void GetLODMaskOverride();                                      //#61
        virtual int GetSortPriority();                                          //#62
        virtual void CopyEvents(Glacier::ZGEOM const*);                         //#63
        virtual Glacier::ZEventBase* FindEvent(char const*);                    //#64
        virtual void* GetEventData(char *);                                     //#65
        virtual void AddEvent(char const*);                                     //#66
        virtual void AttachEvent(Glacier::ZEventBase &);                        //#67
        virtual void EnableClassCall(Glacier::uint);                            //#68
        virtual void DisableClassCall(Glacier::uint);                           //#69
        virtual void SetClassTimerInterval(float);                              //#70
        virtual void CallEvents(int,void *,int, Glacier::ZGEOM*);               //#71
        virtual void SendCommand(Glacier::ZGEOM*, Glacier::ZMSGID, void *);     //#72
        virtual void SendCommand_WRONG_METHOD_0();                              //#73
        virtual void SendCommand_WRONG_METHOD_1();                              //#74
        virtual void SendCommand_WRONG_METHOD_2();                              //#75
        virtual void SendCommandRecursive(Glacier::ZMSGID, void *,
                                          Glacier::ZGEOM *);                    //#76
        virtual void Unknown_Wrong_Method_0000();                               //#77
        virtual void Unknown_Wrong_Method_0001();                               //#78
        virtual void FreeEvents();                                              //#79
        virtual void FreeEvent(Glacier::ZEventBase *);                          //#80
        virtual void Unknown_Wrong_Method_0002();                               //#81
        virtual void Unknown_Wrong_Method_0003();                               //#82
        virtual void RegisterInstance(Glacier::uint);                           //#83 (do nothing)
        virtual void Unknown_Wrong_Method_0004();                               //#84
        virtual void Unknown_Wrong_Method_0005();                               //#85
        virtual void Unknown_Wrong_Method_0006();                               //#86
        virtual void Unknown_Wrong_Method_0007();                               //#87
        virtual void Unknown_Wrong_Method_0008();                               //#88
        virtual void ClassCommand(Glacier::ZMSGID, void *);                     //#89
        virtual void ClassPushPopScene(bool);                                   //#90
        virtual bool ChkLineColi(Glacier::COLI *,bool);                         //#91
        virtual bool CheckPointInside(float const*,float);                      //#92
        virtual bool CheckPointInsideBound(float const*);                       //#93
        virtual bool CheckBoxInside(float const*,float const*,float const*);    //#94
        virtual void GetPointInsideDistance(float const*);                      //#95
        virtual void Visible();                                                 //#96
        virtual void Invisible();                                               //#97
        virtual void PushState();                                               //#98
        virtual void DuplicateData(Glacier::ZGEOM*);                            //#99
        virtual void Duplicate(Glacier::ZGROUP *,char const*,bool);             //#100
        virtual void DuplicateInit(Glacier::ZGROUP*,float const*,float const*,
                                   char const*,bool);                           //#101
        virtual void DuplicateToResource(Glacier::ZGROUP*,Glacier::uint,
                                         char const*,bool);                     //#102
        virtual void DuplicateToResourceInit(Glacier::ZGROUP*,Glacier::uint,
                                             float const*,float const*,
                                             char const*,bool);                 //#103
        virtual void CopyData(Glacier::ZGEOM const*);                           //#104
        virtual void AddActionMap(Glacier::SInputActionDefinition *,char *);    //#105
        virtual void RemoveActionMap(char *);                                   //#106
        virtual void CheckAction(int,void *);                                   //#107
        virtual void CheckAction(char *,void *);                                //#108
        virtual void CheckAction(Glacier::SInputAction *,void *);               //#109
        virtual void GetActionID(char *);                                       //#110
        virtual Glacier::SInputAction GetActionPtr(int);                        //#111
        virtual void* GetActionPtr(char *);                                     //#112
        virtual bool IsAttached(void);                                          //#113
        virtual void SetAttachedTo(Glacier::ZLNKOBJ *);                         //#114
        virtual Glacier::ZLNKOBJ* GetAttachedTo();                              //#115
        virtual bool IsLinkBaseObj();                                           //#116
        virtual void OnCameraEnter();                                           //#117
        virtual void IsRecursiveActivateAllowed();                              //#118
        virtual void DynamicGroupOnScreen();                                    //#119
        virtual bool CheckBoxInside(float const*,float const*,
                                    float const*) const;                        //#120
        virtual void* FindLoadWorldGeom(char const*) const;                     //#121
        virtual Glacier::ZGEOM* FindMasterGeom(char const*) const;              //#122
        virtual Glacier::ZGEOM* FindGeom(char const*, Glacier::ZBaseGeom*);     //#123
        virtual int GroupDepth();                                               //#124
        virtual float GetPFResMultiplier() const;                               //#125
        virtual void LinkBound(unsigned int);                                   //#126
        virtual void RemoveBound(unsigned int);                                 //#127
        virtual void GetAmbientSettings(float const*, float*,
                                        float*, float*) const;                  //#128
        virtual void CreateParentsRecur(Glacier::ZGROUP*, Glacier::ZGROUP**,
                                        bool);                                  //#129
        virtual void SetOverRideNearFar(float*);                                //#130
        virtual void CorrectCenSizeRecur();                                     //#131
        virtual void CorrectCenSize();                                          //#132
        virtual void InvalidateBounds();                                        //#133
        virtual void AttachGeom(Glacier::ZBaseGeom*, bool);                     //#134
        virtual void AttachGeom(Glacier::ZGEOM*, bool);                         //#135
        virtual void DetachGeom(Glacier::ZBaseGeom*, bool);                     //#136
        virtual void RecurGetNextGroup(Glacier::ZBaseGeom const**) const;       //#137
        virtual void RecurGetNextExclRoom(Glacier::ZBaseGeom const**) const;    //#138
        virtual void SetGroupControl(unsigned int, unsigned int);               //#139
        virtual void* GroupControl() const;                                     //#140
        virtual void ResetGroupPosition(bool);                                  //#141
        virtual void MakeActiveRecursive();                                     //#142
        virtual void GetStaticLights(Glacier::ZBaseGeom**,
                                     Glacier::ZBaseGeom**);                     //#143
        virtual void CalcCenSizeRecur();                                        //#144
        virtual void GetCenSizeRecur(float*, float*, bool);                     //#145
        virtual void FindMaskGeom(char const*, int) const;                      //#146
        virtual ZWINDOWS* GetSystem();                                          //#147
        virtual void WndMessage(Glacier::ZWMEVENT*);                            //#148
        virtual void GetMouseColi(Glacier::SMouseColi*, float*, float*);        //#149
        virtual void RecalcMaxMin();                                            //#150
        virtual void CalcRealCenSize(float*, float*) const;                     //#151
        virtual void SetText(char const*);                                      //#152
        virtual void SetAlpha(int);                                             //#153
        virtual void SetLineSpacing(int);                                       //#154
        virtual void Unknown_Virtual_Function_0000();                           //#155
        virtual void Unknown_Virtual_Function_0001();                           //#156
        virtual void Notify(Glacier::ZWMEVENT*, ZWINGROUP*);                    //#157
        virtual void NotifySubscribers(Glacier::ZWMEVENT*);                     //#158
        virtual void CheckCommands();                                           //#159
        virtual void Unknown_Virtual_Function_0002();                           //#160
        virtual void Unknown_Virtual_Function_0003();                           //#161
        virtual void Unknown_Virtual_Function_0004();                           //#162
        virtual void PushWindow(unsigned int);                                  //#163
        virtual void PopWindow();                                               //#164
        virtual void PushWindow(ZWINDOW*);                                      //#165
        virtual void SetMousePos(Glacier::ZVector2*);                           //#166
        virtual void GetMousePos(Glacier::ZVector2*);                           //#167
        virtual void SetShowMouse(bool);                                        //#168
        virtual void UpdateMouse();                                             //#169
        virtual void SetMouseSpeed(float);                                      //#170
        virtual void SetMousePosition(Glacier::ZVector2*);                      //#171
        virtual void SetMouseDeltaPos(Glacier::ZVector2*);                      //#172
        virtual Glacier::ZGEOM* GetMouseGeom() const;                           //#173
        virtual void ForceUpdateMouse();                                        //#174
        virtual void AddCamera(Glacier::ZCAMERA*);                              //#175
        virtual void RemoveCamera(Glacier::ZCAMERA*);                           //#176
        virtual void SetUseZBuffer(bool);                                       //#177
        virtual void SetClearScreen(bool);                                      //#178
        virtual void SetBackColor(int);                                         //#179
        virtual void SetBackColor(int r, int g, int b);                         //#180
        virtual void SetGfxHandler(ZWinGfxHandler*);                            //#181
        virtual ZWinGfxHandler* GetGfxHandler() const;                          //#182
        virtual ZWINDOWS::EventSubscriber* AddSubscriber(unsigned int,
                                                         _evdata, ZWINGROUP*,
                                                         float, bool);          //#183
        virtual void RemoveSubscriber(unsigned int, _evdata, ZWINGROUP*);       //#184

        /// === members ===
        Glacier::ZBaseGeom* m_baseGeom; //0x04
        int field_8;
        int field_c;
        int field_10;
        int field_14;
        int field_18;
        int field_1c;
        int field_20;
        int field_24;
        int field_28;
        int field_2c;
        int field_30;
        int field_34;
        int field_38;
        int field_3c;
        int field_40;
        int field_44;
        int field_48;
        int field_4c;
        int field_50;
        int field_54;
        int field_58;
        int field_5c;
        int field_60;
        int field_64;
        int field_68;
        int field_6c;
        int field_70;
        int field_74;
        int field_78;
        int field_7c;
        int field_80;
        int field_84;
        Glacier::LINKSORTREFTAB m_subscribers;
        Glacier::LINKREFTAB m_linkRefTab_b4;
        Glacier::REFTAB32 m_reftab32;
        int field_188;
        int field_18c;
        Glacier::LINKSORTREFTAB m_cameras;
        int field_1bc;
        ZWinGfxHandler* m_gfxHandler;
        int field_1c4;
        int field_1c8;
        int field_1cc;
        int field_1d0;
        int field_1d4;
        int field_1d8;
        char field_1dc;
        char field_1DD;
        bool m_mouseVisible;
        char field_1DF;
        bool m_shouldClearScreen;
        bool m_forceUpdateMouse;
        char field_1E2;
        char field_1E3;
        int field_1e4;
        char field_1e8;
        char field_1E9;
        char field_1EA;
        char field_1EB;
        int field_1ec;
        ZWinInput m_winInput;
        int field_360;
        int field_364;
        int field_368;
    };
}