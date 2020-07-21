# Build options
setup_build_opts() {
	# The options are separated by '|', the format is as follows
	# <bazel build args> | <binary cli arguments>
	OPTIONS=(
		"|"
		"|--node_host ${NODE_HOST}"
		"|--node_port ${NODE_PORT}"
		"|--ta_host ${TA_HOST}"
		"|--db_host ${DB_HOST}"
		"|--quiet"
		"--define db=enable|"
		"--define build_type=debug|"
		"--define build_type=profile|"
	)
	success=()
	fail=()
}

# Set sanitizer options
setup_sanitizer_opts() {
	SAN_OPTIONS=(
		"--config=asan"
		"--config=tsan"
		"--config=ubsan"
	)
	success=()
	fail=()
}

# Check environment variables
check_env() {
	ENV_NAME=(
		"NODE_HOST"
		"NODE_PORT"
		"TA_HOST"
		"TA_PORT"
		"DB_HOST"
	)

	echo "Checking environment variables"
	echo "=============================="

	for ((i = 0; i < ${#ENV_NAME[@]}; i++)); do
		name=${ENV_NAME[${i}]}
		if [[ -z ${!name} ]]; then
			echo "${name} not set"
			fail=1
		else
			echo "${name} is set to ${!name}"
		fi
	done

	echo "=============================="

	[ -z ${fail} ] || exit 1
}

# Parse command line arguments
get_cli_args() {
	socket=$1
	shift
	remaining_args=$@ # Get the remaining arguments
}

start_notification="TA-START"
