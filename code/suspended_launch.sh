#!/bin/bash
kill -STOP $$
exec "$@"
jobs -l

#kill -CONT <pid>

