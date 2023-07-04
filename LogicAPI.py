import json
import random
import time
import sys
import select

from Main import build_world_graphs, place_items
from Settings import Settings
from SettingsList import logic_tricks
from Search import Search
from Goals import replace_goal_names
from Utils import local_path

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
    # Unix-only stdin detection
    use_stdin = False
    if select.select([sys.stdin, ], [], [], 0.0)[0]:
        use_stdin = True

    if use_stdin:
        # Read plando json from stdin.
        # This also includes a ":collect" key to control
        # whether or not to collect location items or just
        # visit them, useful for keeping some logic rules
        # false in ALR for testing.
        world_conf = json.loads(sys.stdin.read())
    else:
        # Use plando from settings.sav if stdin not used.
        # Otherwise tell the user to specify a plando.
        settingsFile = local_path('settings.sav')
        with open(settingsFile, encoding='utf-8') as f:
            settings = json.load(f)
        try:
            with open(settings['distribution_file'], encoding='utf-8') as f:
                world_conf = json.load(f)
        except Exception as ex:
            print('Specify a plando file to load using the OOTR GUI')
            raise ex

    # Minimum randomizer functions to build a traversable
    # world graph. This will randomly shuffle items that
    # aren't plando'd! For checking logic, use full spoiler
    # logs minus the item pool section as input to prevent
    # random variance.
    worlds = get_reachable_entities(world_conf['settings'])
    s = Search([world.state for world in worlds])

    # Collect some starting items that are left uncollected for
    # spoiler log readability since we don't care about the log
    s.collect_pseudo_starting_items()

    # Get list of all locations instead of just advancement locations
    # to allow auditing logic independent of shuffle settings and
    # item fill
    locs = worlds[0].get_locations()
    s.visit_locations(locs)

    # Send location rule metadata to stdout as a JSON-formatted string
    logic_output = '{\n'
    for loc in worlds[0].get_locations():
        logic_output += (
            f'"{loc.name}": {{\n'
            f'"name": "{loc.name}",\n'
            f'"rule_string": "{loc.rule_string}",\n'
            f'"transformed_rule": "{loc.transformed_rule}",\n'
            f'"visited": {str(s.visited(loc)).lower()},\n'
            f'"child_access_rule": {str(loc.access_rule(s.state_list[0], age="child", spot=loc)).lower()},\n'
            f'"adult_access_rule": {str(loc.access_rule(s.state_list[0], age="adult", spot=loc)).lower()}\n'
            f'}},\n'
        )
    logic_output = logic_output[:-2] + '\n}'
    print(logic_output)
    sys.stdout.flush()
