#!/bin/sh

gem build sqlited.gemspec
bundle exec rake install
