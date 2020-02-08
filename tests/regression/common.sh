# Build options
setup_build_opts() {
    # The options are separated by '|', the format is as follows
    # <bazel build args> | <binary cli arguments> 
    OPTIONS=(
        "|"
        "|--iri_host ${IRI_HOST}"
        "|--iri_port ${IRI_PORT}"
        "|--ta_host ${TA_HOST}"
        "|--db_host ${DB_HOST}"
        "|--verbose"
        "|--proxy_passthrough"
        "--define db=enable|"
        "--define build_type=debug|"
        "--define build_type=profile|"
    )
    success=()
    fail=()
}

# Check environment variables
check_env() {
    ENV_NAME=(
        "IRI_HOST" 
        "IRI_PORT" 
        "TA_HOST" 
        "TA_PORT" 
        "DB_HOST" 
    )

    echo "Checking environment variables"
    echo "=============================="

    for (( i = 0; i < ${#ENV_NAME[@]}; i++ )); do
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
get_cli_args () {
    sleep_time=$1
    shift
    remaining_args=$@ # Get the remaining arguments
}
