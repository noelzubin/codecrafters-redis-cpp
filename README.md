[![progress-banner](https://app.codecrafters.io/progress/redis/d4f4e8a1-095b-40d2-aaf0-2359c6ce5f27)](https://app.codecrafters.io/users/noelzubin)

This is a starting point for C solutions to the
["Build Your Own Redis" Challenge](https://codecrafters.io/challenges/redis).

In this challenge, you'll build a toy Redis clone that's capable of handling
basic commands like `PING`, `SET` and `GET`. Along the way we'll learn about
event loops, the Redis protocol and more.

**Note**: If you're viewing this repo on GitHub, head over to
[codecrafters.io](https://codecrafters.io) to try the challenge.

# Passing the first stage

The entry point for your Redis implementation is in `app/server.c`. Study and
uncomment the relevant code, and push your changes to pass the first stage:

```sh
git add .
git commit -m "pass 1st stage" # any msg
git push origin master
```

That's all!

# Stage 2 & beyond

Note: This section is for stages 2 and beyond.

1. Ensure you have `gcc` installed locally
1. Run `./spawn_redis_server.sh` to run your Redis server, which is implemented
   in `app/server.c`.
1. Commit your changes and run `git push origin master` to submit your solution
   to CodeCrafters. Test output will be streamed to your terminal.



### Source walkthroughs

#### Responding to PINGS

The function that handles the ping command.
https://github.com/redis/redis/blob/1e85b89aefe8e7e24a46bd1c8fb251fdab024b74/src/server.c#L4285

The definitions for all the commands are stored in the command table here
https://github.com/redis/redis/blob/1e85b89aefe8e7e24a46bd1c8fb251fdab024b74/src/server.c#L4285
This file is generated by https://github.com/redis/redis/blob/82b82035553cdbaf81983f91e0402edc8de764ab/utils/generate-command-code.py.
which takes the https://github.com/redis/redis/blob/82b82035553cdbaf81983f91e0402edc8de764ab/src/commands/ping.json as input