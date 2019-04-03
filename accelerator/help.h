#define HELP_INSTR                                                            \
  "Tangle-accelerator is a tool provides accelerated IOTA client service. \n" \
  "Close tangle-accelerator with ctrl+C. \n"
#define TA_HOST_INSTR "The host tangle-accelerator uses \n"
#define TA_PORT_INSTR "The port tangle-accelerator uses \n"
#define TA_THREAD_COUNT_INSTR "How many threads are used."
#define IRI_HOST_INSTR "The IRI host tangle-accelerator connects to. \n"
#define IRI_PORT_INSTR "The IRI port tangle-accelerator connects to. \n"
#define MILESTONE_DEPTH_INSTR "How many milestones tangle-accelerator takes. \n"
#define MWM_INSTR "PoW difficulty. \n"
#define SEED_INSTR "The seed has been used for current machine. \n"
#define REDIS_HOST_INSTR "The host redis server uses \n"
#define REDIS_PORT_INSTR "The port redis server uses \n"
#define TA_VERSION_INSTR "The current tangle-accelerator version. \n"
#define SEE_MORE_LINK "https://github.com/DLTcollab/tangle-accelerator \n"

const char* help_full_description = HELP_INSTR
    "\n"
    "Commands:\n"
    "ta-host: " TA_HOST_INSTR "ta-port: " TA_PORT_INSTR
    "ta-thread: " TA_THREAD_COUNT_INSTR "ta-version: " TA_VERSION_INSTR
    "iri-host: " IRI_HOST_INSTR "iri-port: " IRI_PORT_INSTR
    "milestone_depth: " MILESTONE_DEPTH_INSTR "mwm: " MWM_INSTR
    "seed: " SEED_INSTR "redis-host: " REDIS_HOST_INSTR
    "redis-port: " REDIS_PORT_INSTR
    "\n"
    "See more at" SEE_MORE_LINK;