# The activity text and IPC frames. No engine, no Discord client, no game data.

opents_add_test(DiscordPresence
    NAME discordpresence
    SOURCES discordpresence.cpp
    ENGINE
        discordformat.cpp
    DOCTEST
)
