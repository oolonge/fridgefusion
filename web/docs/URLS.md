http://localhost:9000/	Главная страница (HTTP)	
https://localhost:9443/	Главная страница (HTTPS + HTTP/2)
http://localhost:9000/api/v2/health	Health check Main API	
http://localhost:9000/mirror/api/v2/health	Health check Mirror API
http://localhost:9000/api/docs	Swagger документация
http://localhost:9000/mirror/	Зеркало приложения
http://localhost:3000	Grafana (admin/admin)
http://localhost:3100/ready	Loki status
localhost:5434	PostgreSQL Master (R/W)
localhost:5435	PostgreSQL Slave (R/O)
http://localhost:8080-8082	Backend Main (прямой доступ)	
http://localhost:8090-8092	Backend Mirror (прямой доступ)	
