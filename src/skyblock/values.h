//I put all of my values and functions here
#ifndef SKYBLOCK_VALUES_H
#define SKYBLOCK_VALUES_H

//number of debug dummies
#define DEBUG_DUMMIES_NUMBER 6

namespace NSkyb {
    enum{
        //banking
        MAXIMUM_USERNAME_LENGTH = 120,
        MAXIMUM_PASSWORD_LENGTH = 120,
        MINIMUM_USERNAME_LENGTH = 6,
        MINIMUM_PASSWORD_LENGTH = 6,
        MONEY_DIPASIT_AMOUNT = 1000,
        MONEY_WITHDRAW_AMOUNT = 1000,

        PLAYER_MAXIMUM_SCORE = 9999,

        JETPACK_UPGRADE_PRICE = 21,
        JETPACK_UPGRADE_MAX = 4,
        HOOK_UPGRADE_PRICE = 42,
        HOOK_UPGRADE_MAX = 3,
        JUMP_UPGRADE_PRICE = 51,
        JUMP_UPGRADE_MAX = 2,
        EXTRALIFE_UPGRADE_PRICE = 300,
        EXTRALIFE_UPGRADE_MAX = 1,


    };
}
class CSkyb
{
    public:
    bool IsStandardString(const char *pStr);
    bool IsNumberString(const char *pStr);



};

#endif //SKYBLOCK_VALUES_H