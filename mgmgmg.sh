#!/bin/sh

gem build sqlited.gemspec
bundle exec rake install
gem install --local pkg/sqlited-0.1.0.gem
