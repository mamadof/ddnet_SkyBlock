#ifndef SKYBLOCK_VALUES_H
#define SKYBLOCK_VALUES_H

//I put all of my values and functions here


namespace NSkyb {
    enum{
        MAXIMUM_USERNAME_LENGTH = 120,
        MAXIMUM_PASSWORD_LENGTH = 120,
        MINIMUM_USERNAME_LENGTH = 6,
        MINIMUM_PASSWORD_LENGTH = 6,

        MONEY_DIPASIT_AMOUNT = 1000,
        MONEY_WITHDRAW_AMOUNT = 1000,

        PLAYER_MAXIMUM_SCORE = 6969,
    };
}
class CSkyb
{
    public:
    bool IsStandardString(const char *pStr);
    bool IsNumberString(const char *pStr);



};

#endif //SKYBLOCK_VALUES_H