{APE file}
unit fmt_APE;
{$include compilers.inc}

interface
uses wat_api;

function ReadAPE(var Info:tSongInfo):boolean; cdecl;

implementation

uses windows,common,io,tags,srv_format;

const
  defID = $2043414D;
type
(* Old Version ?
  tMonkeyHeader = record
    ID              :dword; { Always "MAC " }
    VersionID       :word;  { Version number * 1000 (3.91 = 3910) }
    CompressionID   :word;  { Compression level code }
    Flags           :word;  { Any format flags }
    Channels        :word;  { Number of channels }
    SampleRate      :dword; { Sample rate (hz) }
    HeaderBytes     :dword; { Header length (without header ID) }
    TerminatingBytes:dword; { Extended data }
    Frames          :dword; { Number of frames in the file }
    FinalSamples    :dword; { Number of samples in the final frame }
    PeakLevel       :dword; { Peak level (if stored) }
    SeekElements    :dword; { Number of seek elements (if stored) }
  end;
*)
  tMonkeyHeader = packed record
    ID               :dword; // should equal 'MAC '
    VersionID        :dword; // version number * 1000 (3.81 = 3810)
    nDescriptorBytes :dword; // descriptor bytes
    nHeaderBytes     :dword; // APEHeader bytes
    nSeekTableBytes  :dword; // bytes of the seek table
    nHeaderDataBytes :dword; // header data bytes (from original file)
    nFrameDataBytes  :dword; // bytes of APE frame data
    nFrameDataBytesHi:dword; // the high order number of APE frame data bytes
    nTerminatingBytes:dword; // the terminating data of the file (w/o tag data)
    cFileMD5:array [0..15] of byte;
  end;
type
  tAPEHeader = packed record
    nCompressionLevel:word;  // the compression level
    nFormatFlags     :word;  // any format flags (for future use)
    nBlocksPerFrame  :dword; // the number of audio blocks in one frame
    nFinalFrameBlocks:dword; // the number of audio blocks in the final frame
    nTotalFrames     :dword; // the total number of frames
    nBitsPerSample   :word;  // the bits per sample (typically 16)
    nChannels        :word;  // the number of channels (1 or 2)
    nSampleRate      :dword; // the sample rate (typically 44100)
  end;

const
  MONKEY_COMPRESSION_FAST       = 1000; // Fast (poor)
  MONKEY_COMPRESSION_NORMAL     = 2000; // Normal (good)
  MONKEY_COMPRESSION_HIGH       = 3000; // High (very good)
  MONKEY_COMPRESSION_EXTRA_HIGH = 4000; // Extra high (best)
const
  MONKEY_FLAG_8_BIT          = 1;  // Audio 8-bit
  MONKEY_FLAG_CRC            = 2;  // New CRC32 error detection
  MONKEY_FLAG_PEAK_LEVEL     = 4;  // Peak level stored
  MONKEY_FLAG_24_BIT         = 8;  // Audio 24-bit
  MONKEY_FLAG_SEEK_ELEMENTS  = 16; // Number of seek elements stored
  MONKEY_FLAG_WAV_NOT_STORED = 32; // WAV header not stored

function ReadAPE(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  hdr:tMonkeyHeader;
  hdr1:tAPEHeader;
  blocks:dword;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  ReadID3v2(f,Info);
  BlockRead(f,hdr ,SizeOf(tMonkeyHeader));
  BlockRead(f,hdr1,SizeOf(tAPEHeader)); //hdr.nHeaderBytes
  if hdr1.nTotalFrames=0 then
    blocks:=0
  else
    blocks:=(hdr1.nTotalFrames-1)*hdr1.nBlocksPerFrame+hdr1.nFinalFrameBlocks;
  Info.khz     :=hdr1.nSampleRate div 1000;
  if hdr1.nSampleRate<>0 then
    Info.total :=blocks div hdr1.nSampleRate;
  Info.channels:=hdr1.nChannels;
//  Info.kbps:=Info.khz*deep*Info.channels/1152
//  Info.kbps:=(blocks*Info.channels*hdr1.nBitsPerSample) div (Info.total*8000);
//  Info.kbps    :=((hdr1.nBitsPerSample div 8)*hdr1.nSamplerate) div 1000;
(* Old version ?
  if (hdr.ID<>DefID) or (hdr.SampleRate=0) or (hdr.Channels=0) then
    exit;
  if (hdr.VersionID>=3900) or
    ((hdr.VersionID>=3800) and
    (hdr.CompressionID=MONKEY_COMPRESSION_EXTRA_HIGH)) then
    tmp:=73728
  else
    tmp:=9216;
  tmp:=(hdr.Frames-1)*tmp+hdr.FinalSamples;
  Info.total   :=tmp div hdr.SampleRate;
  Info.khz     :=hdr.SampleRate div 1000;
  Info.channels:=hdr.Channels;

  Info.kbps:=tmp;//samples
  if      (hdr.Flags and MONKEY_FLAG_8_BIT)<>0 then tmp:=8
  else if (hdr.Flags and MONKEY_FLAG_24_BIT)<>0 then tmp:=24
  else tmp:=16;
  Info.kbps:=((Info.kbps*tmp*hdr.Channels) div Info.Total) div 1000;
*)
  ReadAPEv2(f,Info);
  ReadID3v1(f,Info);
  CloseHandle(f);
  result:=true;
end;

var
  LocalFormatLink:twFormat;

procedure InitLink;
begin
  LocalFormatLink.Next:=FormatLink;

  LocalFormatLink.This.proc :=@ReadAPE;
  LocalFormatLink.This.ext  :='APE';
  LocalFormatLink.This.flags:=0;

  FormatLink:=@LocalFormatLink;
end;

initialization
  InitLink;
end.
