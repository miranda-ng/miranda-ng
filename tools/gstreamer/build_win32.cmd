@echo off

cd /d "%TEMP%"
rd /s /q gstreamer
mkdir gstreamer

call %VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars32.bat 10.0.17763.0

cd /d "%GSTREAMER_ROOT%"
mkdir builddir
meson setup --prefix="%TEMP%\gstreamer" builddir
IF ERRORLEVEL 1 EXIT

meson compile -C builddir
IF ERRORLEVEL 1 EXIT

cd builddir
meson install
cd ..
rd /s /q builddir

cd /d "%TEMP%\gstreamer\bin"
copy ffi-7.dll            %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gio-2.0-0.dll        %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy glib-2.0-0.dll       %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gmodule-2.0-0.dll    %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gobject-2.0-0.dll    %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstapp-1.0-0.dll     %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstaudio-1.0-0.dll   %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstbase-1.0-0.dll    %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstnet-1.0-0.dll     %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstpbutils-1.0-0.dll %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstreamer-1.0-0.dll  %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstrtp-1.0-0.dll     %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstsctp-1.0-0.dll    %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstsdp-1.0-0.dll     %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gsttag-1.0-0.dll     %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstvideo-1.0-0.dll   %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstwebrtc-1.0-0.dll  %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy gstwinrt-1.0-0.dll   %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy intl-8.dll           %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy nice-10.dll          %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy opus-0.dll           %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy orc-0.4-0.dll        %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy srtp2-1.dll          %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer
copy z.dll                %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer

cd /d "%TEMP%\gstreamer\lib\gstreamer-1.0"
copy gstaudioconvert.dll  %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstaudioresample.dll %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstaudiotestsrc.dll  %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstautodetect.dll    %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstcoreelements.dll  %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstdtls.dll          %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstnice.dll          %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstopus.dll          %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstplayback.dll      %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstrtp.dll           %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstrtpmanager.dll    %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstsrtp.dll          %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstwasapi.dll        %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
copy gstwebrtc.dll        %MIRANDA_BUILD_ROOT%\redist\x32\gstreamer\gst_plugins
