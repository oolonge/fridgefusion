# FridgeFusion

Recipe search driven by what is actually in your fridge: it matches recipes
against the ingredients you have and, when something is missing, proposes
substitutes ranked by a similarity coefficient over a base of 250 ingredients.

Built at BMSTU IU7 across three courses — databases, software design and web
development — each rebuilding the same domain on a different architecture.

## Structure

| Path      | Contents                                                        |
|-----------|------------------------------------------------------------------|
| `desktop` | Qt desktop application, layered architecture                     |
| `web`     | Service-oriented version: gateway, auth, core and data services  |
| `docs`    | BPMN process, ER diagram, use cases                              |

## Desktop

Five layers, each a separate static library, dependencies pointing inward:

```
libs/core            # entities and repository interfaces
libs/application     # use cases
libs/infrastructure  # PostgreSQL repositories
libs/presentation    # Qt widgets, MVP
libs/utils           # logging, configuration
```

```sh
cd desktop && qmake && make
```

Tests live in `desktop/tests`; `schemes/` holds the database schema.

## Web

Four services behind a gateway, orchestrated with Docker Compose, plus a REST
API with Swagger UI and a Caddy front end.

```sh
cd web
cp config.json.example config.json
docker compose up -d
```

Credentials in `docker-compose.yml` are placeholders — set `POSTGRES_PASSWORD`
and `JWT_SECRET` in the environment before running anywhere real.

`web/cli` is a console client against the same API.

## Stack

C++17, Qt 6, PostgreSQL, Docker, Caddy, Swagger
