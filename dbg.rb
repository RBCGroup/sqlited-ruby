require 'sqlited'

lited = Sqlited.new('pyon.db')

lited.exec('CREATE TABLE IF NOT EXISTS user(id integer primary key, name text)')

p "name?"
your_name = gets.chomp

stmt = Sqlited::Statement.new(lited, "INSERT INTO user(name) values('#{your_name}')")
stmt.step
stmt.close

stmt2 = Sqlited::Statement.new(lited, "SELECT * FROM user")
while true
  response = stmt2.step
  break unless response
  p response
end
stmt2.close

lited.close
