#include <gtest/gtest.h>

#include <util/base64.h>

#include <memory>

TEST(Base64, CanDecodeThreeCharacters)
{
    const char *expected = "Man";
    const char base64String[] = {'T', 'W', 'F', 'u'};

    char decodedBuffer[4];
    std::memset(decodedBuffer, 0, 4);

    EXPECT_EQ(3, decodeBase64(base64String, decodedBuffer, 4));
    EXPECT_STREQ(expected, decodedBuffer);
}

TEST(Base64, CanDecodeCharactersWithPadding)
{
    const char *expected = "Ma";
    const char base64String[] = {'T', 'W', 'E', '='};

    char decodedBuffer[4];
    std::memset(decodedBuffer, 0, 4);

    EXPECT_EQ(2, decodeBase64(base64String, decodedBuffer, 4));
    EXPECT_STREQ(expected, decodedBuffer);
}

TEST(Base64, CanDecodeSingleCharacterWithPadding)
{
    const char *expected = "M";
    const char base64String[] = {'T', 'Q', '=', '='};

    char decodedBuffer[4];
    std::memset(decodedBuffer, 0, 4);

    EXPECT_EQ(1, decodeBase64(base64String, decodedBuffer, 4));
    EXPECT_STREQ(expected, decodedBuffer);
}

TEST(Base64, CanDecodeArbitrarilyLongSequence)
{
    const char *expected = "light work.";
    const char *base64String = "bGlnaHQgd29yay4=";

    char decodedBuffer[12];
    std::memset(decodedBuffer, 0, 12);

    EXPECT_EQ(11, decodeBase64(base64String, decodedBuffer, 16));
    EXPECT_STREQ(expected, decodedBuffer);
}
