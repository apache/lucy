#! /usr/bin/env ruby

# Simple example on how to call CFC in ruby
require_relative 'Clownfish/CFC'

hierarchy = Clownfish::CFC::Model::Hierarchy.new("autogen")
hierarchy.build
