#pragma once

DECLARE_LOG_CATEGORY_EXTERN(Forested, Log, All);

//is in world
#define IN_WORLD (PLAYING || IN_EDITOR_WORLD)
//is in editor world
#define IN_EDITOR_WORLD TEST_PLAYING(GetWorld(), EWorldType::Editor)
//is playing game
#define PLAYING (PLAYING_EDITOR || PLAYING_STANDALONE)
//is playing game in PIE
#define PLAYING_EDITOR TEST_PLAYING(GetWorld(), EWorldType::PIE)
//is playing game standalone (not in editor)
#define PLAYING_STANDALONE TEST_PLAYING(GetWorld(), EWorldType::Game)
//is playing game standalone (not in editor) with world parameter
#define TEST_PLAYING(World, InWorldType) (World && World->WorldType == InWorldType)

#define LANDSCAPE_OBJECT_CHANNEL ECollisionChannel::ECC_GameTraceChannel1
#define TREE_ACTOR_OBJECT_CHANNEL ECollisionChannel::ECC_GameTraceChannel2
#define WATER_OBJECT_CHANNEL ECollisionChannel::ECC_GameTraceChannel3
#define LOADABLE_OBJECT_CHANNEL ECollisionChannel::ECC_GameTraceChannel4
#define BLOCK_LIGHT_TRACE_CHANNEL ECollisionChannel::ECC_GameTraceChannel5
#define HIT_TRACE_CHANNEL ECollisionChannel::ECC_GameTraceChannel6
#define HOVER_TRACE_CHANNEL ECollisionChannel::ECC_GameTraceChannel7
#define SELECT_TRACE_CHANNEL ECollisionChannel::ECC_GameTraceChannel8

//macros used to define basic logging behavior with varying parameters

#define PRINT_1(x) PRINT_2(x, 2.f)
#define PRINT_2(x, TimeToDisplay) PRINT_3(x, TimeToDisplay, -1)
#define PRINT_3(x, TimeToDisplay, Key) PRINT_4(x, TimeToDisplay, Key, FColor::Cyan)
#define PRINT_4(x, TimeToDisplay, Key, Color) \
	if (GEngine) GEngine->AddOnScreenDebugMessage(Key, TimeToDisplay, Color, x);

//macro used to choose which print function
#define CHOOSE_ARG(x, TimeToDisplay, Key, Color, FUNC, ...) FUNC
#define CHOOSE_PRINT(...) \
	CHOOSE_ARG(__VA_ARGS__, PRINT_4, PRINT_3, PRINT_2, PRINT_1, )

//basic macro to allow printing to screen
#define PRINT(...) CHOOSE_PRINT(__VA_ARGS__)(__VA_ARGS__)

//macro to print an empty line
#define PRINT_EMPTY PRINT("")

//macro used to put messages in the output log
#define LOG(x, ...) \
	UE_LOG(Forested, Log, TEXT(x), ##__VA_ARGS__)
//macro used to put warnings in the output log
#define LOG_WARNING(x, ...) \
	UE_LOG(Forested, Warning, TEXT(x), ##__VA_ARGS__)
//macro used to put errors in the output log
#define LOG_ERROR(x, ...) \
	UE_LOG(Forested, Error, TEXT(x), ##__VA_ARGS__)
//macro used to put errors in the output log and crash the game, only use if absolutely necessary
#define LOG_FATAL(x, ...) \
	UE_LOG(Forested, Fatal, TEXT(x), ##__VA_ARGS__)

//macro used to put messages in the output log (only outputs in editor)
#if WITH_EDITOR
#define LOG_EDITOR(x, ...) \
	LOG(x, ##__VA_ARGS__);
#else
#define LOG_EDITOR(x, ...)
#endif


//macro used to put warnings in the output log (only outputs in editor)
#if WITH_EDITOR
#define LOG_EDITOR_WARNING(x, ...) \
	LOG_WARNING(x, ##__VA_ARGS__);
#else
#define LOG_EDITOR_WARNING(x, ...)
#endif

//macro used to put errors in the output log (only outputs in editor)
#if WITH_EDITOR
#define LOG_EDITOR_ERROR(x, ...) \
	LOG_ERROR(x, ##__VA_ARGS__);
#else
#define LOG_EDITOR_ERROR(x, ...)
#endif

//macro used to put errors in the output log and crash the game, only use if absolutely necessary (only outputs in editor)
#if WITH_EDITOR
#define LOG_EDITOR_FATAL(x, ...) \
	LOG_FATAL(x, ##__VA_ARGS__);
#else
#define LOG_EDITOR_FATAL(x, ...)
#endif

template <typename... Types>
	FORCENOINLINE static void OutputLogMessage(void* CategoryName, ELogVerbosity::Type Verbosity, const void* LogPoint, Types... FormatArgs)
{
	uint8 FormatArgsBuffer[3072];
	uint16 FormatArgsSize = FFormatArgsTrace::EncodeArguments(FormatArgsBuffer, FormatArgs...);
	if (FormatArgsSize)
	{
		OutputLogMessageInternal(LogPoint, FormatArgsSize, FormatArgsBuffer);
	}
}
