export interface AgentInfo {
  id: string;
  name: string;
  database: string;
  ageSeconds: number;
}

export type DisconnectStatus = 'disconnect_requested' | 'not_found';

interface RawAgent {
  id: string;
  name: string;
  database: string;
  age_seconds: number;
}

async function parseJson<T>(response: Response): Promise<T> {
  if (!response.ok) {
    throw new Error(`HTTP ${response.status} ${response.statusText}`);
  }
  return (await response.json()) as T;
}

export async function fetchAgents(): Promise<AgentInfo[]> {
  const data = await parseJson<{agents: RawAgent[]}>(
    await fetch('/api/agents', {headers: {Accept: 'application/json'}}),
  );
  return data.agents.map((agent) => ({
    id: agent.id,
    name: agent.name,
    database: agent.database,
    ageSeconds: agent.age_seconds,
  }));
}

export async function disconnectAgent(id: string): Promise<DisconnectStatus> {
  const data = await parseJson<{status: DisconnectStatus}>(
    await fetch('/agent/disconnect', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({id}),
    }),
  );
  return data.status;
}
