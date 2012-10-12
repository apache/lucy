#! /usr/bin/env ruby

# Simple example on how to call CFC in ruby
require_relative 'Clownfish/CFC'

hierarchy = Clownfish::CFC::Model::Hierarchy.new(:dest => "autogen")
hierarchy.build

core_binding = Clownfish::CFC::Binding::Core.new(:hierarchy => hierarchy, :header => 'foobar', :footer => '')
core_binding.write_all_modified
