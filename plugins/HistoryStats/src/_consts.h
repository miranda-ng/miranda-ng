#if !defined(HISTORYSTATS_GUARD__CONSTS_H)
#define HISTORYSTATS_GUARD__CONSTS_H

#include "stdafx.h"

#if defined(HISTORYSTATS_CONST_DEFINE)
#define CONST_A(nam, txt) extern const char* nam = txt;
#define CONST_W(nam, txt) extern const wchar_t* nam = L##txt;
#else
#define CONST_A(nam, txt) extern const char* nam;
#define CONST_W(nam, txt) extern const wchar_t* nam;
#endif

/*
 * base colors:
 *
 *    outgoing - #FF0000 (red)
 *    incoming - #007F00 (green)
 *    bar      - #0000FF (blue)
 *    i/o line - #BFBFBF (lt grey)
 *    bar line - #00007F (dk blue)
 *    bar back - #BFBFBF (lt grey)
 *
 * base colors (a0x):
 *
 *    outgoing - #E8641B (red)
 *    incoming - #7CC623 (green)
 *    bar      - #0581B4 (blue)
 *    i/o line - #BFBFBF (lt grey)
 *    bar line - #023F59 (dk blue)
 *    bar back - #BFBFBF (lt grey)
 */

namespace con
{
	// protocol names
	CONST_A(ProtoUnknown              , "(Unknown protocol)"    )

	// historystats settings
	CONST_A(ModHistoryStats           , "HistoryStats"          )
	CONST_A(SettVersion               , "_Version"              )
	CONST_A(SettLastPage              , "_LastPage"             )
	CONST_A(SettShowColumnInfo        , "_ShowColumnInfo"       )
	CONST_A(SettShowSupportInfo       , "_ShowSupportInfo"      )
	CONST_A(SettLastStatisticsFile    , "_LastStatisticsFile"   )
	CONST_A(SettAutoOpenOptions       , "AutoOpenOptions"       )
	CONST_A(SettAutoOpenStartup       , "AutoOpenStartup"       )
	CONST_A(SettAutoOpenMenu          , "AutoOpenMenu"          )
	CONST_A(SettAverageMinTime        , "AverageMinTime"        )
	CONST_A(SettCalcTotals            , "CalcTotals"            )
	CONST_A(SettChatSessionMinDur     , "ChatSessionMinDur"     )
	CONST_A(SettChatSessionTimeout    , "ChatSessionTimeout"    )
	CONST_A(SettColumns               , "Columns"               )
	CONST_A(SettFilterBBCodes         , "FilterBBCodes"         )
	CONST_A(SettFilterRawRTF          , "FilterRawRTF"          )
	CONST_A(SettFilterWords           , "FilterWords"           )
	CONST_A(SettGraphicsMode          , "GraphicsMode"          )
	CONST_A(SettHeaderTooltips        , "HeaderTooltips"        )
	CONST_A(SettHeaderTooltipsIfCustom, "HeaderTooltipsIfCustom")
	CONST_A(SettHideContactMenuProtos , "HideContactMenuProtos" )
	CONST_A(SettIgnoreAfter           , "IgnoreAfter"           )
	CONST_A(SettIgnoreBefore          , "IgnoreBefore"          )
	CONST_A(SettIgnoreOld             , "IgnoreOld"             )
	CONST_A(SettMenuItem              , "MenuItem"              )
	CONST_A(SettMergeContacts         , "MergeContacts"         )
	CONST_A(SettMergeContactsGroups   , "MergeContactsGroups"   )
	CONST_A(SettMergeMode             , "MergeMode"             )
	CONST_A(SettMetaContactsMode      , "MetaContactsMode"      )
	CONST_A(SettNickname              , "Nickname"              )
	CONST_A(SettOmitByRank            , "OmitByRank"            )
	CONST_A(SettOmitByTime            , "OmitByTime"            )
	CONST_A(SettOmitByTimeDays        , "OmitByTimeDays"        )
	CONST_A(SettOmitByValue           , "OmitByValue"           )
	CONST_A(SettOmitByValueData       , "OmitByValueData"       )
	CONST_A(SettOmitByValueLimit      , "OmitByValueLimit"      )
	CONST_A(SettOmitContacts          , "OmitContacts"          )
	CONST_A(SettOmitNumOnTop          , "OmitNumOnTop"          )
	CONST_A(SettOmittedInTotals       , "OmittedInTotals"       )
	CONST_A(SettOmittedInExtraRow     , "OmittedInExtraRow"     )
	CONST_A(SettOnStartup             , "OnStartup"             )
	CONST_A(SettOutput                , "Output"                )
	CONST_A(SettOutputExtraFolder     , "OutputExtraFolder"     )
	CONST_A(SettOutputExtraToFolder   , "OutputExtraToFolder"   )
	CONST_A(SettOutputVariables       , "OutputVariables"       )
	CONST_A(SettOverwriteAlways       , "OverwriteAlways"       )
	CONST_A(SettPathToBrowser         , "PathToBrowser"         )
	CONST_A(SettPNGMode               , "PNGMode"               )
	CONST_A(SettProtosIgnore          , "ProtosIgnore2"         )
	CONST_A(SettRemoveEmptyContacts   , "RemoveEmptyContacts"   )
	CONST_A(SettRemoveInChatsZero     , "RemoveInChatsZero"     )
	CONST_A(SettRemoveInBytesZero     , "RemoveInBytesZero"     )
	CONST_A(SettRemoveOutChatsZero    , "RemoveOutChatsZero"    )
	CONST_A(SettRemoveOutBytesZero    , "RemoveOutBytesZero"    )
	CONST_A(SettShowContactMenu       , "ShowContactMenu"       )
	CONST_A(SettShowContactMenuPseudo , "ShowContactMenuPseudo" )
	CONST_A(SettShowMenuSub           , "ShowMenuSub"           )
	CONST_A(SettSort                  , "Sort"                  )
	CONST_A(SettTableHeader           , "TableHeader"           )
	CONST_A(SettTableHeaderRepeat     , "TableHeaderRepeat"     )
	CONST_A(SettTableHeaderVerbose    , "TableHeaderVerbose"    )
	CONST_A(SettThreadLowPriority     , "ThreadLowPriority"     )
	CONST_A(SettWordDelimiters        , "WordDelimiters"        )
	CONST_A(SettExclude               , "Exclude"               )

	// clist module settings
	CONST_A(ModCList               , "CList"              )
	CONST_A(SettGroup              , "Group"              )

	// column tags
	CONST_W(ColChatDuration        , "chatduration"       )
	CONST_W(ColEvents              , "events"             )
	CONST_W(ColGroup               , "group"              )
	CONST_W(ColInOut               , "inouttext"          )
	CONST_W(ColInOutGraph          , "inout"              )
	CONST_W(ColNick                , "nick"               )
	CONST_W(ColProtocol            , "protocol"           )
	CONST_W(ColRank                , "rank"               )
	CONST_W(ColSplit               , "split"              )
	CONST_W(ColSplitTimeline       , "splittimeline"      )
	CONST_W(ColTimeline            , "timeline"           )
	CONST_W(ColWordCount           , "wordcount"          )
	CONST_W(ColWords               , "commonwords"        )

	// suffix for column-specific settings
	CONST_W(SuffixData             , "/data"              )

	// suffix for shared column data (filter words)
	CONST_W(SuffixWords            , "/words"             )

	// keys for common column settings
	CONST_W(KeyGUID                , "guid"               )
	CONST_W(KeyEnabled             , "enabled"            )
	CONST_W(KeyTitle               , "title"              )

	// keys for column-specific settings
	CONST_W(KeyAbsolute            , "absolute"           ) // InOut, InOutGraph
	CONST_W(KeyAbsTime             , "abs_time"           ) // InOut, InOutGraph
	CONST_W(KeyBlocks              , "blocks"             ) // Split, SplitTimeline
	CONST_W(KeyBlockUnit           , "block_unit"         ) // Split, SplitTimeline
	CONST_W(KeyContactCount        , "contact_count"      ) // Nick
	CONST_W(KeyCustomGroup         , "custom_group"       ) // SplitTimeline
	CONST_W(KeyDays                , "days"               ) // Timeline
	CONST_W(KeyDetail              , "detail"             ) // ChatDuration, InOutGraph, Nick, Split, Timeline, WordCount, Words
	CONST_W(KeyDetailInOut         , "detail_inout"       ) // Words
	CONST_W(KeyDetailInvert        , "detail_invert"      ) // InOutGraph
	CONST_W(KeyDetailPercent       , "detail_percent"     ) // InOutGraph
	CONST_W(KeyDOWGroup            , "dow_group"          ) // SplitTimeline
	CONST_W(KeyFilterLinks         , "filter_links"       ) // [Base]Words
	CONST_W(KeyFilterWords         , "filter_words"       ) // [Base]Words
	CONST_W(KeyGraph               , "graph"              ) // ChatDuration
	CONST_W(KeyGraphAlign          , "graph_align"        ) // Split, SplitTimeline
	CONST_W(KeyGraphPercent        , "graph_percent"      ) // InOutGraph
	CONST_W(KeyHODGroup            , "hod_group"          )	// SplitTimeline
	CONST_W(KeyIgnoreOld           , "ignore_old"         ) // Timeline
	CONST_W(KeyInOutColor          , "in_out_color"       ) // Words
	CONST_W(KeyMaxLength           , "max_length"         ) // [Base]Words
	CONST_W(KeyMinLength           , "min_length"         ) // [Base]Words
	CONST_W(KeyNum                 , "num"                ) // Words, **keys for common column settings**
	CONST_W(KeyOffset              , "offset"             ) // Words
	CONST_W(KeyShowSum             , "show_sum"           ) // InOutGraph
	CONST_W(KeySource              , "source"             ) // [Base]Words, Events, InOut, InOutGraph, Split, SplitTimeline, Timeline
	CONST_W(KeySourceType          , "source_type"        ) // Split, SplitTimeline, Timeline
	CONST_W(KeyTopPerColumn        , "top_per_column"     ) // SplitTimeline
	CONST_W(KeyUnitsPerBlock       , "units_per_block"    ) // Split, SplitTimeline
	CONST_W(KeyVisMode             , "vis_mode"           ) // ChatDuration, Split, SplitTimeline, WordCount, Words

	// keeys for shared column data (filter words)
	CONST_W(KeyID                  , "id"                 )
	CONST_W(KeyName                , "name"               )
	CONST_W(KeyMode                , "mode"               )
	CONST_W(KeyNumWords            , "num_words"          )

	// keys for sort settings
	CONST_W(KeyAsc                 , "asc"                )
	CONST_W(KeyBy                  , "by"                 )

	// miranda services created by historystats
	CONST_A(SvcConfigure           , "HistoryStats/Configure"    )
	CONST_A(SvcCreateStatistics    , "HistoryStats/CreateFile"   )
	CONST_A(SvcShowStatistics      , "HistoryStats/ShowFile"     )
	CONST_A(SvcToggleExclude       , "HistoryStats/ToggleExclude")
#if defined(HISTORYSTATS_HISTORYCOPY)
	CONST_A(SvcHistoryCopy         , "HistoryStats/HistoryCopy"  )
	CONST_A(SvcHistoryPaste        , "HistoryStats/HistoryPaste" )
#endif

	// min/max time
	const uint32_t MinDateTime = 0x00000000;
	const uint32_t MaxDateTime = 0xFFFFFFFF;

	// default colors for html output
	const COLORREF ColorOut     = RGB(0xE8, 0x64, 0x1B);
	const COLORREF ColorIn      = RGB(0x7C, 0xC6, 0x23);
	const COLORREF ColorBar     = RGB(0x05, 0x81, 0xB4);

	const COLORREF ColorIOLine  = RGB(0xBF, 0xBF, 0xBF);
	const COLORREF ColorBarLine = RGB(0x02, 0x3F, 0x59);
	const COLORREF ColorBarBack = RGB(0xBF, 0xBF, 0xBF);

	const COLORREF ColorBack    = RGB(0xFF, 0xFF, 0xFF);
	const COLORREF ColorBorder  = RGB(0x7F, 0x7F, 0x7F);
	const COLORREF ColorHeader  = RGB(0xDF, 0xDF, 0xDF);
	const COLORREF ColorOmitted = RGB(0xDF, 0xDF, 0xDF);
	const COLORREF ColorTotals  = RGB(0xDF, 0xDF, 0xDF);
}

#undef CONST_W
#undef CONST_A
#undef CONST_W

#endif // HISTORYSTATS_GUARD__CONSTS_H