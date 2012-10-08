unit zwrapper;

interface

function ZDecompressBuf(const inBuffer: Pointer; inSize: Integer; out outBuffer: Pointer; out outSize: Integer; outEstimate: Integer): Integer;

implementation

uses zlib;

function ZDecompressBuf(const inBuffer: Pointer; inSize: Integer; out outBuffer: Pointer; out outSize: Integer; outEstimate: Integer): Integer;
var
  zstream           : TZStreamRec;
  delta             : Integer;
begin
  FillChar(zstream, SizeOf(TZStreamRec), 0);

  delta := (inSize + 255) and not 255;

  if outEstimate = 0 then outSize := delta
  else outSize := outEstimate;
  Result := Z_OK;
  GetMem(outBuffer, outSize);
  try
    zstream.next_in := inBuffer;
    zstream.avail_in := inSize;
    zstream.next_out := outBuffer;
    zstream.avail_out := outSize;

    Result := InflateInit(zstream);
    if Result < 0 then Exit;

    try
      Result := inflate(zstream, Z_NO_FLUSH);
      if Result < 0 then Exit;

      while (Result <> Z_STREAM_END) do begin
        Inc(outSize, delta);
        ReallocMem(outBuffer, outSize);

        zstream.next_out := {$IFDEF FPC}PBytef{$ENDIF}(pByte(outBuffer) + zstream.total_out);
        zstream.avail_out := delta;
        Result := inflate(zstream, Z_NO_FLUSH);
        if Result < 0 then Exit;
      end;
    finally
      inflateEnd(zstream);
    end;

    ReallocMem(outBuffer, zstream.total_out);
    outSize := zstream.total_out;

  finally
    if Result < 0 then FreeMem(outBuffer);
  end;
end;

end.