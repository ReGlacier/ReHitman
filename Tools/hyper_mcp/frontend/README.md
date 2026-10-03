# Hyper MCP Router — console frontend

Retrowave-free, clean admin console for the Hyper MCP router, built with
[Gravity UI](https://gravity-ui.com/) (`@gravity-ui/uikit`) + React + TypeScript,
bundled by Vite. The Python `hyper_router.py` serves the build output and answers
the JSON/agent APIs the console talks to.

## Requirements

- Node.js `^20.19.0` or `>=22.12.0` (Vite 7 requirement)
- npm 10+

## Install

```bash
npm install
```

## Development

Run the router (it serves the API and the agent endpoints) and Vite dev server
side by side. Vite proxies `/api`, `/agent` and `/health` to `http://127.0.0.1:8765`
(see `vite.config.ts`).

```bash
# terminal 1 — from Tools/hyper_mcp
python hyper_router.py

# terminal 2 — from Tools/hyper_mcp/frontend
npm run dev
```

Then open http://localhost:5173. Edit files under `src/` for hot reload.

## Production

Build the static bundle, then just run the router — it serves `dist/` automatically:

```bash
npm run build          # outputs to dist/
python ../hyper_router.py
```

Open http://localhost:8765. If `dist/` is missing the server replies with a hint to
run `npm run build`.

`dist/` is git-ignored; rebuild after changing the frontend.

## Endpoints used

| Method | Path                | Purpose                                             |
| ------ | ------------------- | --------------------------------------------------- |
| GET    | `/api/agents`       | List active agents (`{agents: [{id,name,database,age_seconds}]}`) |
| POST   | `/agent/disconnect` | Kick an agent (`{id}` → `{status}`); client reconnects |

## Structure

```
src/
  main.tsx              # React entry, imports Gravity UI styles + app styles
  App.tsx               # Header, auto-refresh polling, theme toggle, grid/empty states
  api.ts                # fetchAgents / disconnectAgent typed client
  components/
    AgentCard.tsx       # One agent card with the "Отключить" action
  styles.css            # Layout/theme-aware styles built on Gravity UI CSS variables
```

## Scripts

- `npm run dev` — Vite dev server with API proxy
- `npm run build` — `tsc -b` typecheck + production bundle to `dist/`
- `npm run preview` — preview the production build
- `npm run typecheck` — type-check only
