#include "Config.h"

nlohmann::json makeDefaultConfigJson() {
    return pl::config::defaultJson(LeviVisionConfig{});
}

nlohmann::json makeConfigSchemaJson() {
    return pl::config::schema(LeviVisionConfig{});
}
