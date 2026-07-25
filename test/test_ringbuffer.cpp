#include <Arduino.h>
#include <unity.h>

#include "../src/system/ringbuffer.h"
#include "../src/transcription/parser.h"

namespace {

void testRingBufferPreservesSampleOrder() {
    echolens::system::AudioRingBuffer buffer(8);
    const int16_t input[] = {10, 20, 30, 40};
    int16_t output[4] = {};

    TEST_ASSERT_EQUAL_UINT32(4, buffer.write(input, 4));
    TEST_ASSERT_EQUAL_UINT32(4, buffer.read(output, 4));
    TEST_ASSERT_EQUAL_INT16_ARRAY(input, output, 4);
}

void testRingBufferDropsWholeFrameOnOverrun() {
    echolens::system::AudioRingBuffer buffer(4);
    const int16_t first[] = {1, 2, 3, 4};
    const int16_t second[] = {5, 6};

    TEST_ASSERT_EQUAL_UINT32(4, buffer.write(first, 4));
    TEST_ASSERT_EQUAL_UINT32(0, buffer.write(second, 2));
    TEST_ASSERT_EQUAL_UINT32(2, buffer.takeDroppedSamples());
}

void testParserExtractsTranscriptResult() {
    constexpr const char kPayload[] =
        R"({"type":"Results","channel":{"alternatives":[{"transcript":"hello world","confidence":0.92}]},"is_final":true,"speech_final":true})";
    const auto result = echolens::transcription::ParseDeepgramResponse(
        reinterpret_cast<const uint8_t*>(kPayload), sizeof(kPayload) - 1);

    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_TRUE(result.isFinal);
    TEST_ASSERT_TRUE(result.speechFinal);
    TEST_ASSERT_EQUAL_STRING("hello world", result.text.c_str());
}

void testParserRejectsMalformedJson() {
    constexpr const char kPayload[] = "{bad json";
    const auto result = echolens::transcription::ParseDeepgramResponse(
        reinterpret_cast<const uint8_t*>(kPayload), sizeof(kPayload) - 1);

    TEST_ASSERT_FALSE(result.valid);
}

}  // namespace

void setup() {
    UNITY_BEGIN();
    RUN_TEST(testRingBufferPreservesSampleOrder);
    RUN_TEST(testRingBufferDropsWholeFrameOnOverrun);
    RUN_TEST(testParserExtractsTranscriptResult);
    RUN_TEST(testParserRejectsMalformedJson);
    UNITY_END();
}

void loop() {}
