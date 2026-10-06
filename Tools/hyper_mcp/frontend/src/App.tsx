import {useCallback, useEffect, useMemo, useRef, useState} from 'react';
import {Button, Icon, Spin, Text, ThemeProvider, Tooltip} from '@gravity-ui/uikit';
import type {RealTheme} from '@gravity-ui/uikit';
import {ArrowsRotateRight, Moon, Server, Sun} from '@gravity-ui/icons';
import {AgentCard} from './components/AgentCard';
import type {KickState} from './components/AgentCard';
import {disconnectAgent, fetchAgents} from './api';
import type {AgentInfo} from './api';

const REFRESH_MS = 4000;
const GHOST_TTL_MS = 5000;
const THEME_KEY = 'hyper-theme';

interface KickEntry {
  state: KickState;
  ts: number;
}

function loadTheme(): RealTheme {
  return localStorage.getItem(THEME_KEY) === 'dark' ? 'dark' : 'light';
}

export function App() {
  const [theme, setTheme] = useState<RealTheme>(loadTheme);
  const [agents, setAgents] = useState<AgentInfo[]>([]);
  const [kick, setKick] = useState<Record<string, KickEntry>>({});
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(true);
  const [updatedAt, setUpdatedAt] = useState<number | null>(null);
  const cache = useRef<Record<string, AgentInfo>>({});

  const toggleTheme = useCallback(() => {
    setTheme((prev) => {
      const next: RealTheme = prev === 'dark' ? 'light' : 'dark';
      localStorage.setItem(THEME_KEY, next);
      return next;
    });
  }, []);

  const load = useCallback(async () => {
    try {
      const list = await fetchAgents();
      for (const agent of list) {
        cache.current[agent.id] = agent;
      }
      setAgents(list);
      setError(null);
      setUpdatedAt(Date.now());
      setKick((prev) => {
        const live = new Set(list.map((agent) => agent.id));
        const now = Date.now();
        const next: Record<string, KickEntry> = {};
        for (const [id, entry] of Object.entries(prev)) {
          if (entry.state === 'pending') {
            next[id] = entry;
          } else if (!live.has(id) && now - entry.ts <= GHOST_TTL_MS) {
            next[id] = entry;
          }
        }
        return next;
      });
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    void load();
    const timer = window.setInterval(() => void load(), REFRESH_MS);
    return () => window.clearInterval(timer);
  }, [load]);

  const handleDisconnect = useCallback(
    async (agent: AgentInfo) => {
      setKick((prev) => ({...prev, [agent.id]: {state: 'pending', ts: Date.now()}}));
      try {
        const status = await disconnectAgent(agent.id);
        setKick((prev) => ({
          ...prev,
          [agent.id]: {state: status === 'not_found' ? 'gone' : 'kicked', ts: Date.now()},
        }));
        void load();
      } catch (err) {
        setError(err instanceof Error ? err.message : String(err));
        setKick((prev) => {
          const next = {...prev};
          delete next[agent.id];
          return next;
        });
      }
    },
    [load],
  );

  const ghosts = useMemo(() => {
    const live = new Set(agents.map((agent) => agent.id));
    return Object.entries(kick)
      .filter(([id, entry]) => entry.state !== 'pending' && !live.has(id))
      .map(([id]) => cache.current[id])
      .filter((agent): agent is AgentInfo => Boolean(agent));
  }, [agents, kick]);

  const showLoader = loading && agents.length === 0;
  const showEmpty = !loading && agents.length === 0 && ghosts.length === 0;

  return (
    <ThemeProvider theme={theme}>
      <div className="hyper-app">
        <header className="hyper-header">
          <div>
            <div className="hyper-title">
              <span className="hyper-title__logo">
                <Icon data={Server} size={22} />
              </span>
              <Text variant="display-2" as="span">
                Hyper MCP Router
              </Text>
            </div>
          </div>

          <div className="hyper-header__actions">
            <div className="hyper-meta">
              <Text variant="subheader-3" color="secondary">
                {agents.length} онлайн
              </Text>
              {updatedAt ? (
                <Text variant="caption-2" color="hint">
                  обновлено {new Date(updatedAt).toLocaleTimeString()}
                </Text>
              ) : null}
            </div>
            <Tooltip content="Обновить" placement="bottom">
              <Button view="flat-secondary" size="m" onClick={() => void load()} loading={loading && !showLoader}>
                <Button.Icon side="start">
                  <Icon data={ArrowsRotateRight} />
                </Button.Icon>
                Обновить
              </Button>
            </Tooltip>
            <Tooltip content={theme === 'dark' ? 'Светлая тема' : 'Тёмная тема'} placement="bottom">
              <Button view="flat-secondary" size="m" onClick={toggleTheme}>
                <Button.Icon side="start">
                  <Icon data={theme === 'dark' ? Sun : Moon} />
                </Button.Icon>
              </Button>
            </Tooltip>
          </div>
        </header>

        {error ? (
          <Text as="div" variant="body-2" color="danger" style={{marginBottom: 16}}>
            Не удалось получить данные: {error}
          </Text>
        ) : null}

        {showLoader ? (
          <div style={{display: 'flex', justifyContent: 'center', padding: '80px 0'}}>
            <Spin size="xl" />
          </div>
        ) : showEmpty ? (
          <div style={{textAlign: 'center', padding: '80px 20px'}}>
            <Text as="div" variant="header-2" color="hint">
              Нет подключённых агентов
            </Text>
            <Text as="div" variant="body-2" color="hint" style={{marginTop: 8}}>
              Ожидание регистрации IDA-инстансов…
            </Text>
          </div>
        ) : (
          <div className="hyper-grid">
            {agents.map((agent) => (
              <AgentCard
                key={agent.id}
                agent={agent}
                kick={kick[agent.id]?.state}
                onDisconnect={handleDisconnect}
              />
            ))}
            {ghosts.map((agent) => (
              <AgentCard
                key={`ghost-${agent.id}`}
                agent={agent}
                kick={kick[agent.id]?.state}
                ghost
                onDisconnect={handleDisconnect}
              />
            ))}
          </div>
        )}
      </div>
    </ThemeProvider>
  );
}
