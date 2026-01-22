#include "log.hh"
#include <print>

int main() {
	auto s = *LOG(ERROR)(
		"割れた{}ネオン{}", "💔", "🌃"
	)(
		"ガラス{}の破片{}", "🪟", "💥"
	)(
		"君の嫌いな{}季節{}が来る{}", "😖", "❄️", "🥶"
	)(
		"「もうすぐ{}だと」", "🔜"
	)(
		"呪文{}のように{}繰り返して{}", "🧙", "🤔", "🔂"
	)(
		"何年{}が経つ{}", "🗓️", "⏭️"
	);
	auto t = *LOG(INFO)(
		"二人{}には夢{}があった", "👭", "😶‍🌫️", "😔"
	)(
		"今は凍り{}ついた花{}", "🧊", "🥀"
	);
	std::println("GetLastError = {}", s);
	std::println("GetLastInfo = {} (should be `[none]` because LOG_LEVEL < INFO)", t == "" ? "[none]" : t);
	return 0;
}
