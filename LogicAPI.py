import json
import random
import time
import sys

from Main import build_world_graphs, place_items
from Settings import Settings
from SettingsList import logic_tricks
from Search import Search
from Goals import replace_goal_names

def read_settings(settings_base: dict) -> Settings:

    settings = Settings(settings_base)
    settings.output_settings = False
    settings.update_seed('TESTLOGICAPITEST')

    settings.load_distribution()

    for trick in logic_tricks.values():
        settings.__dict__[trick['name']] = trick['name'] in settings.allowed_tricks

    if not settings.world_count:
        settings.world_count = 1
    elif settings.world_count < 1 or settings.world_count > 255:
        raise Exception('World Count must be between 1 and 255')

    if settings.player_num < 1:
        raise Exception(f'Invalid player num: {settings.player_num}; must be between (1, {settings.world_count})')
    if settings.player_num > settings.world_count:
        raise Exception(f'Player Num is {settings.player_num}; must be between (1, {settings.world_count})')

    settings.remove_disabled()
    random.seed(settings.numeric_seed)
    settings.resolve_random_settings(cosmetic=False)

    return settings


def get_reachable_entities(settings_base):
    settings = read_settings(settings_base)
    worlds = build_world_graphs(settings)
    place_items(settings, worlds)
    for world in worlds:
        world.distribution.configure_effective_starting_items(worlds, world)
    if worlds[0].enable_goal_hints:
        replace_goal_names(worlds)
    return worlds


def benchmark_rules_time(worlds, s):
    start_time = time.time_ns()
    for l in worlds[0].get_locations():
        child_access = l.access_rule(s.state_list[0], age="child", spot=l)
        adult_access = l.access_rule(s.state_list[0], age="adult", spot=l)
    for e in worlds[0].get_entrances():
        child_access = e.access_rule(s.state_list[0], age="child", spot=e)
        adult_access = e.access_rule(s.state_list[0], age="adult", spot=e)
    end_time = time.time_ns()
    print((end_time - start_time) / 1000000000.0)


if __name__ == "__main__":
    world_conf = json.loads(sys.stdin.read())
    worlds = get_reachable_entities(world_conf['settings'])
    s = Search([world.state for world in worlds])
    locs = worlds[0].get_locations()
    s.visit_locations(locs)
    logic_output = '{\n'
    for loc in worlds[0].get_locations():
        logic_output += (
            f'"{loc.name}": {{\n'
            f'"name": "{loc.name}",\n'
            f'"rule_string": "{loc.rule_string}",\n'
            f'"transformed_rule": "{loc.transformed_rule}",\n'
            f'"visited": "{s.visited(loc)}",\n'
            f'"child_access_rule": "{str(loc.access_rule(s.state_list[0], age="child", spot=loc)).lower()}",\n'
            f'"adult_access_rule": "{str(loc.access_rule(s.state_list[0], age="adult", spot=loc)).lower()}"\n'
            f'}},\n'
        )
    logic_output = logic_output[:-2] + '\n}'
    print(logic_output)
    sys.stdout.flush()
