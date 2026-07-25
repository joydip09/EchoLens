#pragma once

// EchoLens firmware version metadata.
// Bump these on every release. Used in boot logs and (future) OTA checks.

namespace echolens::version {

constexpr int kMajor = 0;
constexpr int kMinor = 1;
constexpr int kPatch = 0;

constexpr const char* kString    = "0.1.0";
constexpr const char* kBuildName = "EchoLens";

}  // namespace echolens::version
