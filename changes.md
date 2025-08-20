
## [2025-08-20] - Version 0.2.0
### Added
- `traceroute.cpp` implemented to compute traceroutes.

### Changed
- JSON structure of captured data updated for visualization.
- Sniffer now tracks traceroute hops accurately; each hop has a separate IP.
- Server code updated to work with new sniffer output.
- Header file modified to include both `Sniffer` and `Traceroute` classes.
