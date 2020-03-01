#                                                            vim:ts=2 sw=2 ai:
#
#

class IPRange
	def initialize(ip_from, ip_to, code)
		@ip_from = ip_from
		@ip_to   = ip_to
		@code    = code
	end

	def to_s
		"(#{@ip_from}, #{@ip_to}, '#{@code}')"
	end
end

ranges       = []
countries    = {}
max_name_len = 0

IO.foreach("ip-to-country.csv") do |line|
	ip_from, ip_to, code, name = line.chomp.split(/,/)
	ranges << IPRange.new(ip_from, ip_to, code)
	max_name_len = name.length if name.length > max_name_len
	countries[code] = name
end

tables = <<END_TABLES
CREATE TABLE ip_ranges (
	ip_from      INTEGER UNSIGNED NOT NULL,
	ip_to        INTEGER UNSIGNED NOT NULL,
	country_code CHAR(2) NOT NULL,

	INDEX (ip_from, ip_to),
	INDEX (country_code)
);

CREATE TABLE countries (
	country_code CHAR(2) NOT NULL PRIMARY KEY,
	country      VARCHAR(#{max_name_len}) NOT NULL
);
END_TABLES

File.open("ip-to-country.sql", "w") do |out|
	out.puts  tables
	out.puts  "INSERT INTO ip_ranges VALUES"
	ranges.each { |range| out.puts range }
	out.puts  ";\n"
	out.puts  "INSERT INTO countries VALUES"
	countries.each do |code, name|
		words = []
		name.scan /\w+/ do |word|
			word.downcase!
			word.capitalize! if word != "of"
			words << word
		end
		name = words.join(" ")
		out.print "\n('#{code}', '#{name}')"
	end
	out.puts  ";"
end

