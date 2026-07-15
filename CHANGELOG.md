# Changelog

All notable changes to Montagem Widener are documented here. Format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versioning
follows [Semantic Versioning](https://semver.org/) adapted for a pre-1.0
beta:

- **PATCH** (0.x.y): bug fixes only, no behavior/feature changes
- **MINOR** (0.x.0): new features or notable user-facing changes
- **MAJOR** (1.0.0+): first stable release, then breaking changes only

## [0.2.0] - 2026-07-15

### Fixed
- No per-channel bus layout restriction meant JUCE's default negotiation
  could in theory accept a >2-channel configuration for this plugin's
  matched-in/out-count bus; the mid/side math (and existing runtime guard)
  already assumed exactly 2 channels. Now explicitly restricted to stereo
  at the bus-negotiation level too.
- The wing-bar decoration's max length was hand-calibrated for one fixed
  480px window size; now computed from the actual window/knob size so it
  can't overflow the edge at other sizes (the exact bug this decoration
  was rewritten to avoid, once already, at the old fixed size).

### Added
- Resizable window (360x280 up to 900x700); the knob now scales with the
  available space instead of staying a fixed size.

## [0.1.0] - 2026-07-12

### Added
- First public beta: one-knob bass-protected stereo widener, VST3/AU/
  Standalone on macOS and Windows.
