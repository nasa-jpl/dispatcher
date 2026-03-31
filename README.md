# Dispatcher

A Qt-based ROS 2 widget for starting, stopping, and monitoring both ROS nodes
and arbitrary shell processes.

![dispatcher](doc/dispatcher_simple.png)

`dispatcher` builds its UI from a YAML file. Each configured item is launched in
its own detached tmux session, and the terminal button can attach a
`gnome-terminal` window to that session when available.

## Runtime Parameters

The `dispatcher` executable uses the following ROS parameters:

| Name | Type | Default | Description |
| --- | --- | --- | --- |
| `dispatcher_config_path` | `string` | `""` | Path to the dispatcher YAML file. |
| `initial_configuration` | `string` | `""` | Configuration name to select on startup. |
| `ssh_timeout_sec` | `int` | `10` | Timeout used when building remote SSH commands. |
| `target_loop_rate_hz` | `double` | `100.0` | Main process/status polling rate. |

## YAML Configuration

The top-level dispatcher YAML supports:

| Key | Required | Description |
| --- | --- | --- |
| `workspace` | Yes | Workspace path used by ROS process items before sourcing `install/setup.bash`. |
| `nodes` | Yes | Process and category definitions rendered in the main panel. |
| `configurations` | No | Named runtime configurations. Each entry may be a simple name or a map with `name`, `cmd_prefix`, `environment_variables`, and `icon`. |
| `cmd_prefix` | No | Default command prefix for the implicit `all` configuration. |
| `environment_variables` | No | Default environment variables for the implicit `all` configuration. |
| `hide_unconfigured_processes` | No | If true, items missing in the active configuration are hidden instead of disabled. |
| `scripts` | No | Script button definitions shown in the scripts panel. |
| `variables` | No | Variable selectors used for `$VARIABLE` command substitution. |

### Node Types

Each entry in `nodes` can be:

- A ROS process item. If `type` is omitted, the item is treated as ROS by default.
- A shell process item with `type: shell`.
- A collapsible category with `type: category` and an `items` array.

Common item fields include:

| Key | Description |
| --- | --- |
| `name` | UI label and tmux-session base name. |
| `cmd` | Command used for the implicit `all` configuration. |
| `configurations` | Per-configuration command definitions. |
| `start_checked` | Whether the item starts checked in the UI. Optional, defaults to `false`. |
| `stop_tmux_cmd` | Stop sequence sent to tmux. Optional, defaults to `C-C`. |
| `hostname` / `user` | Optional remote execution target for local commands or configuration entries. |
| `use_cmd_prefix` | Enables or disables command-prefix injection. |
| `use_environment_variables` | Enables or disables environment-variable injection. |
| `attach_on_start` | Opens a terminal automatically after launch. |

ROS process items can additionally define `node_name` plus an optional
`namespace` that defaults to an empty string, or a `ros_nodes` array with the same
monitoring fields for online-state monitoring.

Shell process items use `pgrep` on the item name to infer online state.

### Scripts and Variables

Script entries support:

| Key | Description |
| --- | --- |
| `name` | Button label. |
| `cmd` or `configurations` | Script command definition. |
| `row`, `column` | Grid placement in the scripts panel. |
| `icon` | Optional Qt resource path for the button icon. |
| `use_terminal` | Whether to wrap execution in `gnome-terminal --`. |

Variable entries support:

| Key | Description |
| --- | --- |
| `name` | Variable name used in commands, for example `$FCAT_LOOP_RATE_HZ`. |
| `choices` | Selectable values exposed in the UI. |

## Example

See the sample YAMLs in [`config/`](config/) for supported combinations:

- [`config/example.yaml`](config/example.yaml)
- [`config/example_remote_session.yaml`](config/example_remote_session.yaml)
- [`config/example_variables.yaml`](config/example_variables.yaml)
- [`config/example_no_configurations.yaml`](config/example_no_configurations.yaml)



## Running

From this package directory after building:

```bash
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 run dispatcher dispatcher --ros-args \
  -p dispatcher_config_path:=/path/to/dispatcher/config.yaml \
  -p initial_configuration:=offline
```

If `initial_configuration` is omitted, the first configured entry is used.

The terminal button opens a command like:

```bash
gnome-terminal -t commander -- tmux a -t 3_commander
```

You can attach manually from any terminal. First, you can list them with

```bash
tmux ls
```
For example, for `example_category.yaml` config:
```bash
$ tmux ls
1_commander: 1 windows (created Tue Mar 31 13:00:05 2026)
1_fcat: 1 windows (created Tue Mar 31 13:00:05 2026)
1_ping1_but_this_is_a_very_long_name_to_demonstrate_elided: 1 windows (created Tue Mar 31 13:00:05 2026)
2_ping1_2: 1 windows (created Tue Mar 31 13:00:05 2026)
3_ping2: 1 windows (created Tue Mar 31 13:00:05 2026)
```

Then, attach to the desired session with:

```bash
tmux a -t 1_fcat
```

## Build And Test

Build `dispatcher` as a one-package workspace from this directory:

```bash
source /opt/ros/jazzy/setup.bash
colcon build --base-paths . --packages-select dispatcher \
  --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON
```

Run the gtest suite with:

```bash
source /opt/ros/jazzy/setup.bash
colcon test --base-paths . --packages-select dispatcher
```

To print the collected test results:

```bash
colcon test-result --verbose
```

## GitLab CI

This package includes a package-local pipeline in [`.gitlab-ci.yml`](.gitlab-ci.yml)
that does not depend on the parent workspace. It uses a ROS 2 Jazzy Docker
image and has two stages:

- `build`: package-local `colcon build`
- `test`: package-local `colcon test` and `colcon test-result --verbose`
