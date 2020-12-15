#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/ZHumanBoid.h>

namespace Hitman::BloodMoney
{
    class ZLnkAction;
    class ZHM3Actor;

    class ActorInfos
    {
    public:
        char pad_0000[0x50];		//0x0000
        Glacier::ZEntityLocator* location;	//0x0050
        char pad_0054[12];			//0x0054
        ZHM3Actor* actor;			//0x0060
        char pad_0064[8];			//0x0064
        char* unkType;				//0x006C
        char pad_0070[80];			//0x0070
        uint32_t equipment;			//0x00C0 [ptr]
        char pad_00C4[12];			//0x00C4
        Glacier::ZGEOM* attachment;				//0x00D0 [ptr]
        char pad_00D4[92];			//0x00D4
        uint32_t unk_equ2;			//0x0130 [ptr]
        char pad_0134[12];			//0x0134
    }; //Size: 0x0140

    enum EActorMapRole : int {
        Civilian                 = 0,
        Enemy                    = 1,
        VIP                      = 2,
        Target                   = 3,
        UnknownRole_FilledYellow = 4,
        Police                   = 5,
        Optional                 = 6,
        Default                  = Civilian
    };

    enum class UnknownEnum : int { //From 00637BD0
        Value0 = 0x2000000C,
        Value1 = 0x2000000E,
        Value2 = 0x2000000F,
        Value3 = 0x20000010,
        Value4 = 0x20000011,
        Value5 = 0x20000009,
        Value6 = 0x2000000A,
        Value7 = 0x2000000D,
        Other  = 0x0
    };

    enum class UnknownEnum1 : int {
        Value7 = 7,
        Value8 = 8,
        /// also
        Value5 = 5,
        Value6 = 6,
        Value9 = 9,
        Value10 = 10,
        Value11 = 11,
        Value12 = 12,
        Value13 = 13,
        Value14 = 14,
        Value15 = 15
    };

    class ZHM3Actor
    {
    public:
        // === sub types ===
        enum class ActorState : int32_t {
            // Pure
            Status_0 = 0,
            Status_1 = 1,
            Status_2 = 2,
            Status_3 = 3,
            Reset = 0xFF,
            // User
            Sleep  = Status_2, ///< Actor will not be dead
            Normal = Status_1,  ///< Actor will be in normal state
            StopAI = Status_0, ///< Actor will not follow by scripts, will stay in inactive mode. Also in this mode actor is in god mode
            ResetAI = Status_3 ///< Actor will be resetted. Any damage, screaming or other stuff will be flushed (not in god mode!)
        };

        enum class SuiteMask : int32_t {
            NoActor					 = 0b0000'0000'0000'0000'0000'0000'0000'0000,	///< This is not an actor (M13, wheelchair gui subactor kind)
            SkinChangerNotSupported	 = 0b0000'0000'0000'0000'0000'0000'0000'0001,	///< This actor does not supports skin changer
            Nude					 = 0b0000'0000'0000'0000'0000'0000'0000'0010,	///< Nude view of model (works only if the actor can share their suit, in other case works as Inivisble)
            Invisible				 = 0b0000'0000'0000'0000'0000'0000'0000'0100,	///< Invisible view of the actor (possible model loading failure, I don't know)
            OriginalView			 = 0b0000'0000'0000'0000'0000'0000'0000'0101,	///< Just original view of actor
            Agent47_WithoutHeaddress = 0b0000'0000'0000'0000'0000'0000'0000'0110,	///< Sometimes works if suit have headdress (in other case will work as invisible)
            Agent47_WithHeaddress	 = 0b0000'0000'0000'0000'0000'0000'0000'0111	///< Works in 99% of situations with actors who can share their suit
        };

        // === vftable ===
        virtual void Function_0000(); //#0 +0 .rdata:00796334
        virtual void Function_0001(); //#1 +4 .rdata:00796338
        virtual void Function_0002(); //#2 +8 .rdata:0079633c
        virtual void Function_0003(); //#3 +c .rdata:00796340
        virtual void loadParameters(Glacier::ZPackedInput* params); //#4 +10 .rdata:00796344
        virtual void Function_0005(); //#5 +14 .rdata:00796348
        virtual void Function_0006(); //#6 +18 .rdata:0079634c
        virtual void Function_0007(); //#7 +1c .rdata:00796350
        virtual void Function_0008(); //#8 +20 .rdata:00796354
        virtual void Function_0009(); //#9 +24 .rdata:00796358
        virtual void Function_0010(); //#10 +28 .rdata:0079635c
        virtual void Function_0011(); //#11 +2c .rdata:00796360
        virtual void Function_0012(); //#12 +30 .rdata:00796364
        virtual void Function_0013(); //#13 +34 .rdata:00796368
        virtual void Function_0014(); //#14 +38 .rdata:0079636c
        virtual void Function_0015(); //#15 +3c .rdata:00796370
        virtual void Function_0016(); //#16 +40 .rdata:00796374
        virtual void Function_0017(); //#17 +44 .rdata:00796378
        virtual void Function_0018(); //#18 +48 .rdata:0079637c
        virtual void Function_0019(); //#19 +4c .rdata:00796380
        virtual void Function_0020(); //#20 +50 .rdata:00796384
        virtual void Function_0021(); //#21 +54 .rdata:00796388
        virtual void Function_0022(); //#22 +58 .rdata:0079638c
        virtual void Function_0023(); //#23 +5c .rdata:00796390
        virtual void Function_0024(); //#24 +60 .rdata:00796394
        virtual void Function_0025(); //#25 +64 .rdata:00796398
        virtual void Function_0026(); //#26 +68 .rdata:0079639c
        virtual void Function_0027(); //#27 +6c .rdata:007963a0
        virtual void Function_0028(); //#28 +70 .rdata:007963a4
        virtual void Function_0029(); //#29 +74 .rdata:007963a8
        virtual void Function_0030(); //#30 +78 .rdata:007963ac
        virtual void Function_0031(); //#31 +7c .rdata:007963b0
        virtual void Function_0032(); //#32 +80 .rdata:007963b4
        virtual void Function_0033(); //#33 +84 .rdata:007963b8
        virtual void Function_0034(); //#34 +88 .rdata:007963bc
        virtual void Function_0035(); //#35 +8c .rdata:007963c0
        virtual void Function_0036(); //#36 +90 .rdata:007963c4
        virtual void Function_0037(); //#37 +94 .rdata:007963c8
        virtual void Function_0038(); //#38 +98 .rdata:007963cc
        virtual void Function_0039(); //#39 +9c .rdata:007963d0
        virtual void Function_0040(); //#40 +a0 .rdata:007963d4
        virtual void Function_0041(); //#41 +a4 .rdata:007963d8
        virtual void Function_0042(); //#42 +a8 .rdata:007963dc
        virtual void Function_0043(); //#43 +ac .rdata:007963e0
        virtual void Function_0044(); //#44 +b0 .rdata:007963e4
        virtual void Function_0045(); //#45 +b4 .rdata:007963e8
        virtual void Function_0046(); //#46 +b8 .rdata:007963ec
        virtual void Function_0047(); //#47 +bc .rdata:007963f0
        virtual void Function_0048(); //#48 +c0 .rdata:007963f4
        virtual void Function_0049(); //#49 +c4 .rdata:007963f8
        virtual void Function_0050(); //#50 +c8 .rdata:007963fc
        virtual void Function_0051(); //#51 +cc .rdata:00796400
        virtual void Function_0052(); //#52 +d0 .rdata:00796404
        virtual void Function_0053(); //#53 +d4 .rdata:00796408
        virtual void Function_0054(); //#54 +d8 .rdata:0079640c
        virtual void Function_0055(); //#55 +dc .rdata:00796410
        virtual void Function_0056(); //#56 +e0 .rdata:00796414
        virtual void Function_0057(); //#57 +e4 .rdata:00796418
        virtual void Function_0058(); //#58 +e8 .rdata:0079641c
        virtual void Function_0059(); //#59 +ec .rdata:00796420
        virtual void Function_0060(); //#60 +f0 .rdata:00796424
        virtual void Function_0061(); //#61 +f4 .rdata:00796428
        virtual void Function_0062(); //#62 +f8 .rdata:0079642c
        virtual void Function_0063(); //#63 +fc .rdata:00796430
        virtual Glacier::ZEventBase* FindEvent(const char* name); //#64 +100 .rdata:00796434
        virtual void Function_0065(); //#65 +104 .rdata:00796438
        virtual void Function_0066(); //#66 +108 .rdata:0079643c
        virtual void Function_0067(); //#67 +10c .rdata:00796440
        virtual void Function_0068(); //#68 +110 .rdata:00796444
        virtual void Function_0069(); //#69 +114 .rdata:00796448
        virtual void Function_0070(); //#70 +118 .rdata:0079644c
        virtual void Function_0071(); //#71 +11c .rdata:00796450
        virtual void sendEvent(std::uintptr_t ev, int unk0, int unk1); //#72 +120 .rdata:00796454
        virtual void Function_0073(); //#73 +124 .rdata:00796458
        virtual void Function_0074(); //#74 +128 .rdata:0079645c
        virtual void Function_0075(); //#75 +12c .rdata:00796460
        virtual void Function_0076(); //#76 +130 .rdata:00796464
        virtual void Function_0077(); //#77 +134 .rdata:00796468
        virtual void Function_0078(); //#78 +138 .rdata:0079646c
        virtual void Function_0079(); //#79 +13c .rdata:00796470
        virtual void Function_0080(); //#80 +140 .rdata:00796474
        virtual void Function_0081(); //#81 +144 .rdata:00796478
        virtual void Function_0082(); //#82 +148 .rdata:0079647c
        virtual void Function_0083(); //#83 +14c .rdata:00796480
        virtual void Function_0084(); //#84 +150 .rdata:00796484
        virtual void Function_0085(); //#85 +154 .rdata:00796488
        virtual void Function_0086(); //#86 +158 .rdata:0079648c
        virtual void processAI(); //#87 +15c .rdata:00796490
        virtual void Function_0088(); //#88 +160 .rdata:00796494
        virtual void dropAnimation(int, int); //#89 +164 .rdata:00796498
        virtual void Function_0090(); //#90 +168 .rdata:0079649c
        virtual void Function_0091(); //#91 +16c .rdata:007964a0
        virtual void Function_0092(); //#92 +170 .rdata:007964a4
        virtual void Function_0093(); //#93 +174 .rdata:007964a8
        virtual void Function_0094(); //#94 +178 .rdata:007964ac
        virtual void Function_0095(); //#95 +17c .rdata:007964b0
        virtual void Function_0096(); //#96 +180 .rdata:007964b4
        virtual void Function_0097(); //#97 +184 .rdata:007964b8
        virtual void Function_0098(); //#98 +188 .rdata:007964bc
        virtual void Function_0099(); //#99 +18c .rdata:007964c0
        virtual void Function_0100(); //#100 +190 .rdata:007964c4
        virtual void Function_0101(); //#101 +194 .rdata:007964c8
        virtual void Function_0102(); //#102 +198 .rdata:007964cc
        virtual void Function_0103(); //#103 +19c .rdata:007964d0
        virtual void Function_0104(); //#104 +1a0 .rdata:007964d4
        virtual void Function_0105(); //#105 +1a4 .rdata:007964d8
        virtual void Function_0106(); //#106 +1a8 .rdata:007964dc
        virtual void Function_0107(); //#107 +1ac .rdata:007964e0
        virtual void Function_0108(); //#108 +1b0 .rdata:007964e4
        virtual void Function_0109(); //#109 +1b4 .rdata:007964e8
        virtual void Function_0110(); //#110 +1b8 .rdata:007964ec
        virtual void Function_0111(); //#111 +1bc .rdata:007964f0
        virtual void Function_0112(); //#112 +1c0 .rdata:007964f4
        virtual void Function_0113(); //#113 +1c4 .rdata:007964f8
        virtual void Function_0114(); //#114 +1c8 .rdata:007964fc
        virtual void Function_0115(); //#115 +1cc .rdata:00796500
        virtual void Function_0116(); //#116 +1d0 .rdata:00796504
        virtual void Function_0117(); //#117 +1d4 .rdata:00796508
        virtual void Function_0118(); //#118 +1d8 .rdata:0079650c
        virtual void Function_0119(); //#119 +1dc .rdata:00796510
        virtual void Function_0120(); //#120 +1e0 .rdata:00796514
        virtual void Function_0121(); //#121 +1e4 .rdata:00796518
        virtual void Function_0122(); //#122 +1e8 .rdata:0079651c
        virtual void Function_0123(); //#123 +1ec .rdata:00796520
        virtual void Function_0124(); //#124 +1f0 .rdata:00796524
        virtual void Function_0125(Glacier::ZAnimationInfo* _anim, int _flag, float a0, float a1, float a2); //#125 +1f4 .rdata:00796528
        virtual Glacier::ZAnimationInfo** setAnimation(Glacier::ZAnimationInfo* animation, int unknown0 = 1); //#126 +1f8 .rdata:0079652c
        virtual void Function_0127(); //#127 +1fc .rdata:00796530
        virtual void Function_0128(); //#128 +200 .rdata:00796534
        virtual void Function_0129(); //#129 +204 .rdata:00796538
        virtual void Function_0130(); //#130 +208 .rdata:0079653c
        virtual void Function_0131(); //#131 +20c .rdata:00796540
        virtual void Function_0132(); //#132 +210 .rdata:00796544
        virtual void Function_0133(); //#133 +214 .rdata:00796548
        virtual void Function_0134(); //#134 +218 .rdata:0079654c
        virtual void Function_0135(); //#135 +21c .rdata:00796550
        virtual void Function_0136(); //#136 +220 .rdata:00796554
        virtual void Function_0137(); //#137 +224 .rdata:00796558
        virtual void Function_0138(); //#138 +228 .rdata:0079655c
        virtual void Function_0139(); //#139 +22c .rdata:00796560
        virtual void Function_0140(); //#140 +230 .rdata:00796564
        virtual void Function_0141(); //#141 +234 .rdata:00796568
        virtual void Function_0142(); //#142 +238 .rdata:0079656c
        virtual void Function_0143(); //#143 +23c .rdata:00796570
        virtual void Function_0144(); //#144 +240 .rdata:00796574
        virtual void Function_0145(); //#145 +244 .rdata:00796578
        virtual void Function_0146(); //#146 +248 .rdata:0079657c
        virtual void Function_0147(); //#147 +24c .rdata:00796580
        virtual void Function_0148(); //#148 +250 .rdata:00796584
        virtual void Function_0149(); //#149 +254 .rdata:00796588
        virtual void Function_0150(); //#150 +258 .rdata:0079658c
        virtual void Function_0151(); //#151 +25c .rdata:00796590
        virtual void Function_0152(); //#152 +260 .rdata:00796594
        virtual void Function_0153(); //#153 +264 .rdata:00796598
        virtual void Function_0154(); //#154 +268 .rdata:0079659c
        virtual void Function_0155(); //#155 +26c .rdata:007965a0
        virtual void Function_0156(); //#156 +270 .rdata:007965a4
        virtual void Function_0157(); //#157 +274 .rdata:007965a8
        virtual void Function_0158(); //#158 +278 .rdata:007965ac
        virtual void Function_0159(); //#159 +27c .rdata:007965b0
        virtual void Function_0160(); //#160 +280 .rdata:007965b4
        virtual void Function_0161(); //#161 +284 .rdata:007965b8
        virtual void Function_0162(); //#162 +288 .rdata:007965bc
        virtual void Function_0163(); //#163 +28c .rdata:007965c0
        virtual void Function_0164(); //#164 +290 .rdata:007965c4
        virtual void Function_0165(); //#165 +294 .rdata:007965c8
        virtual void Function_0166(); //#166 +298 .rdata:007965cc
        virtual void Function_0167(); //#167 +29c .rdata:007965d0
        virtual void Function_0168(); //#168 +2a0 .rdata:007965d4
        virtual void Function_0169(); //#169 +2a4 .rdata:007965d8
        virtual void Function_0170(); //#170 +2a8 .rdata:007965dc
        virtual void Function_0171(); //#171 +2ac .rdata:007965e0
        virtual void Function_0172(); //#172 +2b0 .rdata:007965e4
        virtual void Function_0173(); //#173 +2b4 .rdata:007965e8
        virtual void Function_0174(); //#174 +2b8 .rdata:007965ec
        virtual void Function_0175(); //#175 +2bc .rdata:007965f0
        virtual void Function_0176(); //#176 +2c0 .rdata:007965f4
        virtual void Function_0177(); //#177 +2c4 .rdata:007965f8
        virtual void Function_0178(); //#178 +2c8 .rdata:007965fc
        virtual void Function_0179(); //#179 +2cc .rdata:00796600
        virtual void Function_0180(); //#180 +2d0 .rdata:00796604
        virtual void Function_0181(); //#181 +2d4 .rdata:00796608
        virtual void Function_0182(); //#182 +2d8 .rdata:0079660c
        virtual void Function_0183(); //#183 +2dc .rdata:00796610
        virtual void Function_0184(); //#184 +2e0 .rdata:00796614
        virtual void Function_0185(); //#185 +2e4 .rdata:00796618
        virtual void Function_0186(); //#186 +2e8 .rdata:0079661c
        virtual void Function_0187(); //#187 +2ec .rdata:00796620
        virtual int Function_0188(); //#188 +2f0 .rdata:00796624 {desc=Get +B40 field from thisptr)
        virtual int Function_0189(); //#189 +2f4 .rdata:00796628
        virtual bool Function_0190(); //#190 +2f8 .rdata:0079662c
        virtual void Function_0191(); //#191 +2fc .rdata:00796630
        virtual void Function_0192(int _a0, int _a1, int _a2); //#192 +300 .rdata:00796634 (possible rotate actor or navigate)
        virtual void Function_0193(); //#193 +304 .rdata:00796638 {load anim cache}
        virtual void Function_0194(); //#194 +308 .rdata:0079663c
        virtual void Function_0195(); //#195 +30c .rdata:00796640
        virtual void Function_0196(); //#196 +310 .rdata:00796644
        virtual void Function_0197(); //#197 +314 .rdata:00796648
        virtual void Function_0198(); //#198 +318 .rdata:0079664c
        virtual void Function_0199(); //#199 +31c .rdata:00796650
        virtual void Function_0200(); //#200 +320 .rdata:00796654
        virtual void Function_0201(); //#201 +324 .rdata:00796658
        virtual void Function_0202(); //#202 +328 .rdata:0079665c
        virtual void Function_0203(); //#203 +32c .rdata:00796660
        virtual void Function_0204(); //#204 +330 .rdata:00796664
        virtual void Function_0205(); //#205 +334 .rdata:00796668
        virtual void Function_0206(); //#206 +338 .rdata:0079666c
        virtual void Function_0207(); //#207 +33c .rdata:00796670
        virtual void Function_0208(); //#208 +340 .rdata:00796674
        virtual void Function_0209(); //#209 +344 .rdata:00796678
        virtual void Function_0210(); //#210 +348 .rdata:0079667c
        virtual void Function_0211(); //#211 +34c .rdata:00796680
        virtual void Function_0212(); //#212 +350 .rdata:00796684
        virtual void Function_0213(); //#213 +354 .rdata:00796688
        virtual void Function_0214(); //#214 +358 .rdata:0079668c
        virtual void Function_0215(); //#215 +35c .rdata:00796690
        virtual void Function_0216(); //#216 +360 .rdata:00796694
        virtual void Function_0217(); //#217 +364 .rdata:00796698
        virtual void Function_0218(); //#218 +368 .rdata:0079669c
        virtual void Function_0219(); //#219 +36c .rdata:007966a0
        virtual void Function_0220(); //#220 +370 .rdata:007966a4
        virtual void Function_0221(); //#221 +374 .rdata:007966a8
        virtual void Function_0222(); //#222 +378 .rdata:007966ac
        virtual void Function_0223(); //#223 +37c .rdata:007966b0
        virtual void Function_0224(); //#224 +380 .rdata:007966b4
        virtual void Function_0225(); //#225 +384 .rdata:007966b8
        virtual void Function_0226(); //#226 +388 .rdata:007966bc
        virtual void Function_0227(); //#227 +38c .rdata:007966c0
        virtual ZLnkAction* getActionByEnum(UnknownEnum a0); //#228 +390 .rdata:007966c4
        virtual void Function_0229(); //#229 +394 .rdata:007966c8
        virtual void Function_0230(); //#230 +398 .rdata:007966cc
        virtual void Function_0231(); //#231 +39c .rdata:007966d0
        virtual void Function_0232(); //#232 +3a0 .rdata:007966d4
        virtual void Function_0233(); //#233 +3a4 .rdata:007966d8
        virtual void Function_0234(); //#234 +3a8 .rdata:007966dc
        virtual void Function_0235(); //#235 +3ac .rdata:007966e0
        virtual void Function_0236(); //#236 +3b0 .rdata:007966e4
        virtual void Function_0237(); //#237 +3b4 .rdata:007966e8
        virtual void Function_0238(); //#238 +3b8 .rdata:007966ec
        virtual void Function_0239(int _unk0); //#239 +3bc .rdata:007966f0
        virtual void Function_0240(); //#240 +3c0 .rdata:007966f4
        virtual void Function_0241(int _possibleItemId); //#241 +3c4 .rdata:007966f8
        virtual void Function_0242(); //#242 +3c8 .rdata:007966fc
        virtual void Function_0243(); //#243 +3cc .rdata:00796700
        virtual void Function_0244(); //#244 +3d0 .rdata:00796704
        virtual void Function_0245(); //#245 +3d4 .rdata:00796708
        virtual void Function_0246(); //#246 +3d8 .rdata:0079670c
        virtual bool Function_0247(); //#247 +3dc .rdata:00796710
        virtual void Function_0248(); //#248 +3e0 .rdata:00796714
        virtual void Function_0249(); //#249 +3e4 .rdata:00796718
        virtual void Function_0250(); //#250 +3e8 .rdata:0079671c
        virtual void Function_0251(); //#251 +3ec .rdata:00796720
        virtual void Function_0252(); //#252 +3f0 .rdata:00796724
        virtual void Function_0253(); //#253 +3f4 .rdata:00796728
        virtual void Function_0254(); //#254 +3f8 .rdata:0079672c
        virtual void Function_0255(); //#255 +3fc .rdata:00796730
        virtual void Function_0256(); //#256 +400 .rdata:00796734
        virtual void Function_0257(); //#257 +404 .rdata:00796738
        virtual void Function_0258(); //#258 +408 .rdata:0079673c
        virtual void Function_0259(); //#259 +40c .rdata:00796740
        virtual void Function_0260(UnknownEnum1 _a0, int _itemID); //#260 +410 .rdata:00796744
        virtual const char* Function_0261(); //#261 +414 .rdata:00796748
        virtual void Function_0262(); //#262 +418 .rdata:0079674c
        virtual void Function_0263(); //#263 +41c .rdata:00796750
        virtual void Function_0264(); //#264 +420 .rdata:00796754
        virtual void Function_0265(); //#265 +424 .rdata:00796758
        virtual void Function_0266(); //#266 +428 .rdata:0079675c
        virtual void Function_0267(); //#267 +42c .rdata:00796760
        virtual void Function_0268(); //#268 +430 .rdata:00796764
        virtual void Function_0269(); //#269 +434 .rdata:00796768
        virtual void Function_0270(); //#270 +438 .rdata:0079676c
        virtual void Function_0271(); //#271 +43c .rdata:00796770
        virtual void Function_0272(); //#272 +440 .rdata:00796774
        virtual bool OnlyVerticalAim(); //#273 +444 .rdata:00796778 { always false }
        virtual void GetAimAnim(Glacier::Animation::Header** results, uint32_t& count); //#274 +448 .rdata:0079677c
        virtual void GetPoseEffects(float* ,unsigned short*); //#275 +44c .rdata:00796780 { BAD ASSEMBLY! }
        virtual int SetHandTarget(int unknownIndex, Glacier::Vector3*, float); //#276 +450 .rdata:00796784
        virtual void DontAnimateAttachers(); //#277 +454 .rdata:00796788 { DO NOT NOTHING }
        virtual void RemoveLHandIKTarget(float, void* callback); //#278 +458 .rdata:0079678c
        virtual void RemoveRHandIKTarget(float, void* callback); //#279 +45c .rdata:00796790
        virtual void UpdateAimTarget(); //#280 +460 .rdata:00796794
        virtual int SetAimTarget(uint32_t, Glacier::Vector3*, float, void* callback, int, int, int, int); //#281 +464 .rdata:00796798 (called by sub_6A6750)
        virtual void SetAimTarget2(/* TODO: Detect args! Do not use this function! */); //#282 +468 .rdata:0079679c
        virtual void RemoveAimTarget(float, void* callback); //#283 +46c .rdata:007967a0
        virtual bool FreezeAimTarget(); //#284 +470 .rdata:007967a4
        virtual void DisableAim(); //#285 +474 .rdata:007967a8
        virtual void EnableAim(); //#286 +478 .rdata:007967ac
        virtual bool GetAimTarget(); //#287 +47c .rdata:007967b0
        virtual bool GetAimTarget(uint32_t*); //#288 +480 .rdata:007967b4
        virtual bool GetAimTarget(Glacier::Vector3*); //#289 +484 .rdata:007967b8
        virtual float GetDrawWeaponSpeed(); //#290 +488 .rdata:007967bc
        virtual bool IsAiming(); //#291 +48c .rdata:007967c0
        virtual void SetAimAnimPrc(float, float, float); //#292 +490 .rdata:007967c4 { DO NOTHING I GUESS }
        virtual void FireShotNotify(); //#293 +494 .rdata:007967c8
        virtual void SetHitAnimation(const char*); //#294 +498 .rdata:007967cc
        virtual void SetHitAnimHandle(int& handle); //#295 +49c .rdata:007967d0
        virtual void UseItem(Glacier::ZItem*, int); //#296 +4a0 .rdata:007967d4
        virtual void DropItem(Glacier::ZItem*); //#297 +4a4 .rdata:007967d8
        virtual void PickupItem(Glacier::ZItem*); //#298 +4a8 .rdata:007967dc
        virtual void Reload(Glacier::ZItem*); //#299 +4ac .rdata:007967e0
        virtual void Chamber(Glacier::ZItem*); //#300 +4b0 .rdata:007967e4
        virtual void PerformDrop(Glacier::ZItem*, bool doPhysicsBody, bool a3); //#301 +4b4 .rdata:007967e8
        virtual Glacier::ZIKHAND* GetRHand(); //#302 +4b8 .rdata:007967ec
        virtual Glacier::ZIKHAND* GetLHand(); //#303 +4bc .rdata:007967f0
        virtual Glacier::ZItem* GetLHandItem(); //#304 +4c0 .rdata:007967f4
        virtual Glacier::ZItem* GetRHandItem(); //#305 +4c4 .rdata:007967f8
        virtual Glacier::ZItemWeapon* GetLHandWeapon(); //#306 +4c8 .rdata:007967fc
        virtual Glacier::ZItemWeapon* GetRHandWeapon(); //#307 +4cc .rdata:00796800
        virtual Glacier::ZItemTemplate* GetRHandItemType(); //#308 +4d0 .rdata:00796804
        virtual Glacier::ZItemTemplate* GetLHandItemType(); //#309 +4d4 .rdata:00796808
        virtual void SetRHandItemState(int itemState); //#310 +4d8 .rdata:0079680c
        virtual void SetLHandItemState(int itemState); //#311 +4dc .rdata:00796810
        virtual void AttachItemToHand(Glacier::ZIKHAND* hand, uint32_t itemId); //#312 +4e0 .rdata:00796814
        virtual bool AttachRHandItem(uint32_t itemID); //#313 +4e4 .rdata:00796818
        virtual bool AttachLHandItem(uint32_t itemID); //#314 +4e8 .rdata:0079681c
        virtual void GetNearItems(); //#315 +4ec .rdata:00796820 {similar to next}
        virtual void GetNearItems2(); //#316 +4f0 .rdata:00796824 {similar to prev}
        virtual void SetDialog(bool); //#317 +4f4 .rdata:00796828
        virtual bool GetDialog(); //#318 +4f8 .rdata:0079682c
        virtual void* DialogListen(int variant); //#319 +4fc .rdata:00796830 {variant between 0 and 1}
        virtual void* DialogTalk(int variant); //#320 +500 .rdata:00796834 {variant between 0 and 1}
        virtual void ForceRHandItem(Glacier::ZItem*, bool); //#321 +504 .rdata:00796838
        virtual void ForceLHandItem(Glacier::ZItem*, bool); //#322 +508 .rdata:0079683c {second bool not used I guess}
        virtual void Function_0323(); //#323 +50c .rdata:00796840
        virtual void DestroyRHandItem(); //#324 +510 .rdata:00796844
        virtual void DestroyLHandItem(); //#325 +514 .rdata:00796848
        virtual bool EmptyHands(); //#326 +518 .rdata:0079684c
        virtual void SetAmmoFor(Glacier::ZItemWeapon*, int); //#327 +51c .rdata:00796850 (do nothing)
        virtual int GetAmmoFor(Glacier::ZItemWeapon*); //#328 +520 .rdata:00796854 {always 999}
        virtual void SetLHandIKTarget(const float*, const float*, float, void* callback); //#329 +524 .rdata:00796858
        virtual void SetRHandIKTarget(const float*, const float*, float, void* callback); //#330 +528 .rdata:0079685c
        virtual void UpdateItemActions(); //#331 +52c .rdata:00796860
        virtual void CallBackItemAction(); //#332 +530 .rdata:00796864
        virtual int GetNearItems3(); //#333 +534 .rdata:00796868
        virtual void PlaySpeech(uint32_t, int, bool); //#334 +538 .rdata:0079686c
        virtual void PlayFootStep(int /* EFootSide */); //#335 +53c .rdata:00796870
        virtual bool IsFirstPersonCamera(); //#336 +540 .rdata:00796874
        virtual float GetCombatStrength(); //#337 +544 .rdata:00796878
        virtual void SetActorRootTM(const Glacier::Vector3*, const Glacier::Vector3*); //#338 +548 .rdata:0079687c
        virtual void SetActorPosDir(const Glacier::Vector3*, const Glacier::Vector3*); //#339 +54c .rdata:00796880
        virtual void* GetActorRootTM(const float*, const float*); //#340 +550 .rdata:00796884
        virtual void GetActorWorldPosition(Glacier::Vector3*); //#341 +554 .rdata:00796888 {possible setPosition!!!}
        virtual void SetActorWorldPosition(Glacier::Vector3*); //#342 +558 .rdata:0079688c
        virtual bool CanPlayAnimSegment(Glacier::Animation::Header*, float, float, const float*, const float*, bool, float, float); //#343 +55c .rdata:00796890
        virtual bool IsUnconscious(); //#344 +560 .rdata:00796894
        virtual bool IsAwake(); //#345 +564 .rdata:00796898 {possible isAlive}
        virtual bool IsSleeping(); //#346 +568 .rdata:0079689c
        virtual void SetHero(uint8_t); //#347 +56c .rdata:007968a0
        virtual int SetActorState(ActorState status); //#348 +570 .rdata:007968a4
        virtual void OnViewEnter(); //#349 +574 .rdata:007968a8
        virtual void OnViewLeave(); //#350 +578 .rdata:007968ac
        virtual void* /* TODO: PF4::RunTime */ GetPathFinder4(); //#351 +57c .rdata:007968b0 (return ZEngineDataBase + 0x14)
        virtual bool EnablePathFinder(bool _a0); //#352 +580 .rdata:007968b4
        virtual void SetStopDistance(float ); //#353 +584 .rdata:007968b8
        virtual void SetEndDir(Glacier::ZVector3* pThreeFloats); //#354 +588 .rdata:007968bc (it works with coordinates but not world coordinates [-1.0; 1.0])
        virtual const Glacier::ZVector3* GetEndDir(); //#355 +58c .rdata:007968c0
        virtual void MoveToPosition(const Glacier::Vector3*, const Glacier::Vector3*); //#356 +590 .rdata:007968c4
        virtual void SlideToPosition(const Glacier::Vector3*, const Glacier::Vector3*); //#357 +594 .rdata:007968c8
        virtual void SetPathNotify(float ); //#358 +598 .rdata:007968cc
        virtual void SetPathNotifySyncToCycle(bool arg); //#359 +59c .rdata:007968d0
        virtual void SetEndAction(bool arg); //#360 +5a0 .rdata:007968d4
        virtual void RemovePath(uint8_t,void* PATH_CANCEL_REASONS, bool); //#361 +5a4 .rdata:007968d8
        virtual void SendPathDone(bool); //#362 +5a8 .rdata:007968dc
        virtual void* GetMoveFromPos(const Glacier::Vector3*); //#363 +5ac .rdata:007968e0
        virtual void* FindNextPathBlocker(); //#364 +5b0 .rdata:007968e4
        virtual int SetShootIntoGround(bool arg); //#365 +5b4 .rdata:007968e8
        virtual bool SetShootIntoGroundInFullbody(bool _a0); //#366 +5b8 .rdata:007968ec
        virtual bool SetStayInsidePath(bool _a0); //#367 +5bc .rdata:007968f0
        virtual void ShootIntoGround(float*, bool, bool); //#368 +5c0 .rdata:007968f4
        virtual void ShootIntoGroundRegularly(float*, bool, bool, float); //#369 +5c4 .rdata:007968f8
        virtual float SetMoveSpeedMultiplier(float speedMul); //#370 +5c8 .rdata:007968fc {set 5B0 value}
        virtual void SetPathFinderEnabled(bool); //#371 +5cc .rdata:00796900
        virtual void OnPathRequest(uint32_t); //#372 +5d0 .rdata:00796904
        virtual void OnPathFinished(uint32_t); //#373 +5d4 .rdata:00796908 {possible reset actor state}
        virtual void OnPathCanceled(void* PATH_CANCEL_ERRORS); //#374 +5d8 .rdata:0079690c
        virtual void OnPathCanceledLockedDoor(uint32_t); //#375 +5dc .rdata:00796910
        virtual void OnPathError(void* PATH_ERRORS); //#376 +5e0 .rdata:00796914
        virtual bool OnPathNotify(); //#377 +5e4 .rdata:00796918
        virtual void OnNewPath(); //#378 +5e8 .rdata:0079691c
        virtual void OnSound(Glacier::REFTAB* ); //#379 +5ec .rdata:00796920
        virtual void Die(); //#380 +5f0 .rdata:00796924
        virtual void DieByForce(const Glacier::ZVector3*, const Glacier::ZVector3*, float, int); //#381 +5f4 .rdata:00796928
        virtual int Resurrect(); //#382 +5f8 .rdata:0079692c
        virtual int Knockout(); //#383 +5fc .rdata:00796930 (possible not but actor not dies here)
        virtual int Revive(); //#384 +600 .rdata:00796934 {possible sedate or smth l ths}
        virtual int GetKnockedOut(); //#385 +604 .rdata:00796938
        virtual int GetActorList(); //#386 +608 .rdata:0079693c
        virtual void SetDisableIdleAnimation(bool); //#387 +60c .rdata:00796940
        virtual int GetAudibleRoomList(); //#388 +610 .rdata:00796944
        virtual void LookAt(int _a0); //#389 +614 .rdata:00796948
        virtual bool IsSeeing(); //#390 +618 .rdata:0079694c
        virtual void SetSeeing(bool); //#391 +61c .rdata:00796950
        virtual bool IsVisible(); //#392 +620 .rdata:00796954
        virtual void SetVisible(bool); //#393 +624 .rdata:00796958
        virtual void WantToLookAt(Glacier::ZGEOM*, uint8_t); //#394 +628 .rdata:0079695c
        virtual void VerifyPlayerVisible(); //#395 +62c .rdata:00796960 (sync suspicious level with ZOSD)
        virtual void DeterminePathLookAt(Glacier::LocomotionInfo *,float *,float *,float *,float *); //#396 +630 .rdata:00796964
        virtual bool CanLookAt(Glacier::ZGEOM*); //#397 +634 .rdata:00796968
        virtual bool IsOnStairs(); //#398 +638 .rdata:0079696c
        virtual void UpdatePositionOffScreen(); //#399 +63c .rdata:00796970
        virtual void UpdatePosition(); //#400 +640 .rdata:00796974
        virtual int GetCurrentUBAnim(Glacier::ZItem *, Glacier::ZItem *,bool &); //#401 +644 .rdata:00796978
        virtual Glacier::ZAnimationInfo* SetHoldWeaponUBAnim(Glacier::ZAnimationInfo* _anim); //#402 +648 .rdata:0079697c
        virtual void GetAnimOffset(const char*); //#403 +64c .rdata:00796980
        virtual void GetAnimOffset(Glacier::Animation::Header*); //#404 +650 .rdata:00796984
        virtual Glacier::Animation::Header* GetAnimHeader(uint32_t); //#405 +654 .rdata:00796988
        virtual void Function_0406(); //#406 +658 .rdata:0079698c
        virtual void Function_0407(); //#407 +65c .rdata:00796990
        virtual void Function_0408(); //#408 +660 .rdata:00796994
        virtual void Function_0409(); //#409 +664 .rdata:00796998
        virtual int SetWeapon(std::intptr_t _itemID); //#410 +668 .rdata:0079699c, possible giveItem method
        virtual std::intptr_t GetWeapon(); //#411 +66c .rdata:007969a0
        virtual void SetWeaponTemplate(uint32_t); //#412 +670 .rdata:007969a4
        virtual std::intptr_t GetWeaponTemplate(); //#413 +674 .rdata:007969a8
        virtual int FireWeapon(uint32_t weaponId, bool, float, bool); //#414 +678 .rdata:007969ac
        virtual void Bite(int, Glacier::Vector3*, Glacier::Vector3*, float, float); //#415 +67c .rdata:007969b0 [Works but need to have a bit in hands]
        virtual void Punch(bool); //#416 +680 .rdata:007969b4 [Works but need to determine condition of usage]
        virtual void StunGunAttack(); //#417 +684 .rdata:007969b8 [Works, but need to check that actor has stun gun in hands!]
        virtual int ThrowKnifeAtRef(std::intptr_t _itemID0, std::intptr_t _itemID1); //#418 +688 .rdata:007969bc (knife thrower check, maybe item hooking here)
        virtual void ActivateRagdollOrAnim(); //#419 +68c .rdata:007969c0
        virtual void ActivateRagdollNextFrame(); //#420 +690 .rdata:007969c4
        virtual void InitializeHitpoints(float ); //#421 +694 .rdata:007969c8
        virtual void SetHitpoints(float ); //#422 +698 .rdata:007969cc
        virtual int GetHitpoints(); //#423 +69c .rdata:007969d0
        virtual void TakeDamage(float ); //#424 +6a0 .rdata:007969d4
        virtual float GetHealth(); //#425 +6a4 .rdata:007969d8
        virtual float GetWeaponStrength(); //#426 +6a8 .rdata:007969dc
        virtual bool IsDragEnabled(); //#427 +6ac .rdata:007969e0
        virtual Glacier::ZItemTemplate* GetItemTemplate(); //#428 +6b0 .rdata:007969e4
        virtual void CreateItemFromItemTemplate(int _itemOrItemID); //#429 +6b4 .rdata:007969e8
        virtual void ActivateItem(int _itemID, UnknownEnum1 _unk0); //#430 +6b8 .rdata:007969ec | _unk0 allowed only Value7 and Value8
        virtual bool ConcealItem(int _itemID, bool _flag0); //#431 +6bc .rdata:007969f0
        virtual void EnablePickupClothes(); //#432 +6c0 .rdata:007969f4
        virtual void DisablePickupClothes(); //#433 +6c4 .rdata:007969f8
        virtual bool IsWearingClothes(); //#434 +6c8 .rdata:007969fc
        virtual void OnHitmanChangedClothes(); //#435 +6cc .rdata:00796a00
        virtual std::intptr_t GetHitmanAs(); //#436 +6d0 .rdata:00796a04
        virtual int GetActorProperties(); //#437 +6d4 .rdata:00796a08
        virtual void SetScriptUBAnim(Glacier::Animation::Header*, bool); //#438 +6d8 .rdata:00796a0c
        virtual int  GetDeadBodyFlags(Glacier::ZLNKOBJ*); //#439 +6dc .rdata:00796a10
        virtual void StartDrag(int _itemID); //#440 +6e0 .rdata:00796a14
        virtual void EndDrag(); //#441 +6e4 .rdata:00796a18
        virtual int GetAutoAimTarget(); //#442 +6e8 .rdata:00796a1c
        virtual void SetAutoAimTarget(bool value); //#443 +6ec .rdata:00796a20
        virtual void StoreUBHoldAnims(); //#444 +6f0 .rdata:00796a24 (loadExtendedAnimations)
        virtual int MapUBHoldAnimsToIdx(); //#445 +6f4 .rdata:00796a28
        virtual int GetItemAssignedUBAnim(Glacier::ZItem*  item); //#446 +6f8 .rdata:00796a2c
        virtual void LoadSave(Glacier::ISerializerStream&); //#447 +6fc .rdata:00796a30 (actor flags, maybe not used by game)
        virtual bool ActivateRigidBody(const Glacier::SRigidBodyVelocity*); //#448 +700 .rdata:00796a34

        // === members ===
        ActorInfos* ActorInformation;	//0x0004
        /// ===========================================
        char pad_0008[12]; //0x0008
        uint32_t m_field14; //0x0014
        char pad_0018[52]; //0x0018
        SuiteMask m_suitMask; //0x004C
        char pad_0050[20]; //0x0050
        int32_t m_field64; //0x0064
        char pad_0068[28]; //0x0068
        Glacier::ZBodyInfo* m_bodyInfo; //0x0084
        char pad_0088[240]; //0x0088
        float m_field178; //0x0178
        Glacier::ZActionDispatcher* m_actionDispatcher; //0x017C
        char pad_0180[140]; //0x0180
        int32_t m_field20C; //0x020C
        char pad_0210[12]; //0x0210
        int32_t m_field21C; //0x021C
        char pad_0220[16]; //0x0220
        int32_t m_field230; //0x0230
        char pad_0234[4]; //0x0234
        int32_t m_field238; //0x0238
        char pad_023C[244]; //0x023C
        int32_t m_field330; //0x0330
        char pad_0334[12]; //0x0334
        int32_t m_field340; //0x0340
        char pad_0344[108]; //0x0344
        int32_t m_field3B0; //0x03B0
        char pad_03B4[12]; //0x03B4
        int32_t m_field3C0; //0x03C0
        char pad_03C4[44]; //0x03C4
        int32_t m_field3F0; //0x03F0
        Glacier::ZHumanBoid* m_boid; //0x03F4
        char pad_03F8[68]; //0x03F8
        float m_field43C; //0x043C
        float m_field440; //0x0440
        float m_field444; //0x0444
        char pad_0448[24]; //0x0448
        int32_t m_field460; //0x0460
        char pad_0464[28]; //0x0464
        int32_t m_field480; //0x0480
        int32_t m_field484; //0x0484
        char pad_0488[4]; //0x0488
        int32_t m_field48C; //0x048C
        char pad_0490[256]; //0x0490
        float m_field590; //0x0590
        float m_field594; //0x0594
        float m_field598; //0x0598
        char pad_059C[68]; //0x059C
        int32_t m_field5E0; //0x05E0
        char pad_05E4[8]; //0x05E4
        float m_field5EC; //0x05EC
        float m_field5F0; //0x05F0
        float m_field5F4; //0x05F4
        char pad_05F8[736]; //0x05F8
        int32_t m_field8D8; //0x08D8
        char pad_08DC[4]; //0x08DC
        int32_t m_field8E0; //0x08E0
        int32_t m_field8E4; //0x08E4
        uint32_t m_animCurrent; //0x08E8
        char pad_08EC[20]; //0x08EC
        EActorMapRole m_actorRole; //0x0900
        int32_t m_HmAsID; //0x0904
        int32_t m_STDOBJID; //0x0908
        int32_t m_field90C; //0x090C
        int32_t m_field910; //0x0910
        int32_t m_field914; //0x0914
        char pad_0918[56]; //0x0918
        float m_field950; //0x0950
        char pad_0954[4]; //0x0954
        float m_field958; //0x0958
        char pad_095C[4]; //0x095C
        float m_field960; //0x0960
        char pad_0964[4]; //0x0964
        float m_field968; //0x0968
        char pad_096C[8]; //0x096C
        float m_field974; //0x0974
        float m_field978; //0x0978
        char pad_097C[88]; //0x097C
        int32_t m_field9D4; //0x09D4
        int32_t m_field9D8; //0x09D8
        char pad_09DC[20]; //0x09DC
        uint32_t m_animHoldToolbox; //0x09F0
        uint32_t m_animHoldBriefcase; //0x09F4
        char pad_09F8[4]; //0x09F8
        uint32_t m_animHoldCoffee; //0x09FC
        char pad_0A00[12]; //0x0A00
        uint32_t m_animHoldBeer; //0x0A0C
        char pad_0A10[44]; //0x0A10
        float m_fieldA3C; //0x0A3C
        float m_fieldA40; //0x0A40
        char pad_0A44[132]; //0x0A44
        Glacier::ZROOM* m_room; //0x0AC8
        float m_fieldACC; //0x0ACC
        char pad_0AD0[8]; //0x0AD0
        int32_t m_lastLogicUpdateTick; //0x0AD8
        char pad_0ADC[32]; //0x0ADC
        int32_t m_fieldAFC; //0x0AFC
        char pad_0B00[12]; //0x0B00
        float m_fieldB0C; //0x0B0C
        int32_t m_fieldB10; //0x0B10
        char pad_0B14[56]; //0x0B14
        int32_t m_fieldB4C; //0x0B4C
        char pad_0B50[16]; //0x0B50
    }; //Size: 0x0008
}