# @cubicleai/wrtc

[![NPM](https://img.shields.io/npm/v/@cubicleai/wrtc.svg)](https://www.npmjs.com/package/@cubicleai/wrtc)

Node.js bindings for `libwebrtc`, which implements [WebRTC M96](https://groups.google.com/g/discuss-webrtc/c/Bp8OzBzipSc/m/0AC4OGhdAgAJ). This project aims for spec-compliance and is tested using the W3C's [web-platform-tests](https://github.com/web-platform-tests/wpt) project. A number of [nonstandard APIs](docs/nonstandard-apis.md) for testing are also included.

# Install

```
npm install @cubicleai/wrtc
```

Installing from NPM downloads a prebuilt binary for your operating system × architecture. Set the `TARGET_ARCH` environment variable to "arm" or "arm64" to download for armv7l or arm64, respectively. Linux and macOS users can also set the `DEBUG` environment variable to download debug builds.

You can also [build from source](docs/build-from-source.md).

# Supported Platforms

We intend to officially support
- the latest 3 stable versions of Node.js 
- the latest 3 stable releases of Electron 

For the following platforms:
- Linux
- macOS
- Windows

On the following architectures:
- x64
- arm64
- armv7l 

Build validation is not yet in place for all of these platforms. 

The following platforms are confirmed to work with `@cubicleai/wrtc`. Some may have prebuilt binaries available. Since we target [N-API version 3](https://nodejs.org/api/n-api.html), there may be additional platforms supported that are not listed here. If your platform is not supported, you may still be able to [build from source](docs/build-from-source.md).

The table below maps our support intentions to which configurations have been validated.
<table>
  <thead>
    <tr>
      <td style="text-align: center;" colspan="2" rowspan="2"></td>
      <th style="text-align: center;" colspan="3">Linux</th>
      <th style="text-align: center;" colspan="2">macOS</th>
      <th style="text-align: center;">Windows</th>
    </tr>
    <tr>
      <th style="text-align: center;">armv7l</th>
      <th style="text-align: center;">arm64</th>
      <th style="text-align: center;">x64</th>
      <th style="text-align: center;">x64</th>
      <th style="text-align: center;">arm64</th>
      <th style="text-align: center;">x64</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <th rowspan="3">Node.js</th>
      <th>14</th>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center">✓</td>
      <td align="center"></td>
      <td align="center">✓</td>
      <td align="center">✓</td>
    </tr>
    <tr>
      <th>16</th>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center">✓</td>
      <td align="center"></td>
      <td align="center">✓</td>
      <td align="center">✓</td>
    </tr>
    <tr>
      <th>18</th>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center">✓</td>
      <td align="center"></td>
      <td align="center">✓</td>
      <td align="center">✓</td>
    </tr>
    <tr>
      <th rowspan="3">Electron</th>
      <th>18</th>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
    </tr>
    <tr>
      <th>19</th>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
    </tr>
    <tr>
      <th>20</th>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
      <td align="center"></td>
    </tr>
  </tbody>
</table>

## Platform-specific details

### Linux

Other libwebrtc bindings use Google's in-tree `libc++` when targetting Linux. Due to compatibility issues, and to reduce the need for multiple C++ standard libraries to be loaded within a single application when using `@cubicleai/wrtc` in concert with other native add-ons, we have opted to use the system's `libstdc++` instead. This does mean that you will need a compatible version of `libstdc++` installed when you use the prebuilt versions of the library. If the prebuilt versions give you runtime link errors related to missing versions of `libstdc++`, either install the correct version or use `SKIP_DOWNLOAD=true` when running `npm install` to build from source instead.

# Examples

See [cubicleai/webrtc-examples](https://github.com/cubicleai/webrtc-examples).

# Development

Make sure to check the platform-specific sections below for important information.

To set up a working copy for development:

## Linux/Mac

Pre-steps
- Install `python3`, C/C++ toolchain (ie `build-essential`), `cmake`

```shell
export SKIP_DOWNLOAD=true   # Important to skip fetching a prebuilt version from CDN
export DEBUG=1
export PARALLELISM=24       # Set to number of logical cores on your machine

# Initial install will build libwebrtc
# Get a coffee.

npm install
```

## Windows

Pre-steps:
- Install python3
- Make sure long path support is enabled
    - Set `HKLM\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnable` to `1`
    - `git config --global core.longpaths true`
    - `git config core.longpaths true`
- Install VS 2019 (not just the VS 2019 build tools from a newer VS release like 2022)
    - `libwebrtc` is the limiting factor here
- Install Windows 10 SDK v10.0.19041.0
    * Use the Windows SDK installer, make sure to include the required Debugging Tools for Windows
    * (!!) Do not use the Visual Studio installer, if you have previously installed this SDK via Visual Studio Installer, 
      you must first remove it and install using the Windows SDK installer instead. If you use this, the build will fail
      on requirement of Windows SDK 10.0.19041.0

Initial build

```powershell
$env:SKIP_DOWNLOAD = 'true'   # Important to skip fetching a prebuilt version from CDN
$env:DEBUG = '1'
$env:PARALLELISM = '24'       # set to number of logical cores

# Initial install will build libwebrtc
# Get a coffee.

npm install
```

# Required Reading

References
- [RTP: A Transport Protocol for Real-Time Applications](https://datatracker.ietf.org/doc/html/rfc3550) (IETF RFC 3550)
- [Unified Plan](https://datatracker.ietf.org/doc/html/draft-roach-mmusic-unified-plan-00#section-2)
- [Plan B](https://datatracker.ietf.org/doc/html/draft-uberti-rtcweb-plan-00)
- [RTCRtpTransceiver](https://developer.mozilla.org/en-US/docs/Web/API/RTCRtpTransceiver) (MDN)
- [Negotiating Media Multiplexing Using the Session Description Protocol](https://datatracker.ietf.org/doc/html/draft-ietf-mmusic-sdp-bundle-negotiation-54#section-18) ("BUNDLE" IETF I-D)

Editorial
- [The evolution of WebRTC 1.0.](https://blog.mozilla.org/webrtc/the-evolution-of-webrtc/) (Mozilla)
- [Exploring RTCRtpTransceiver.](https://blog.mozilla.org/webrtc/rtcrtptransceiver-explored/) (Mozilla)
- [Webrtc with transceivers](https://niccoloterreri.com/webrtc-with-transceivers) (nterreri)