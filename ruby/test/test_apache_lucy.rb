require 'test/unit'
require 'apache_lucy'

class ApacheLucyTest < Test::Unit::TestCase
  def test_lucy_version
    assert Lucy::VERSION
  end
end

