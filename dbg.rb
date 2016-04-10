require 'sqlited'

lited = Sqlited.new('pyon.db')

lited.sqlited_exec('CREATE TABLE IF NOT EXISTS user(id integer primary key, name text)')

p "name?"
your_name = gets.chomp

lited.prepare_sql("INSERT INTO user(name) values('#{your_name}')")
lited.step

lited.prepare_sql("SELECT * FROM user")
while true
  response = lited.step
  break unless response
  p response
end

lited.close
